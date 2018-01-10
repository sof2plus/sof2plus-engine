/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors
Copyright (C) 2017, SoF2Plus contributors

This file is part of the SoF2Plus source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 3 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
// g2_collision.c - Code for collision detection for models game-side.

#include "g2_local.h"

/*
=============================================
-----------------------------
Vertex convenience functions.
-----------------------------
=============================================
*/

static ID_INLINE int G2_GetVertWeights(const mdxmVertex_t *pVert)
{
    // 1..4 count
    int iNumWeights = (pVert->uiNmWeightsAndBoneIndexes >> 30)+1;

    return iNumWeights;
}

static ID_INLINE int G2_GetVertBoneIndex(const mdxmVertex_t *pVert, const int iWeightNum)
{
    int boneIndex = (pVert->uiNmWeightsAndBoneIndexes >> (iG2_BITS_PER_BONEREF * iWeightNum)) & ((1 << iG2_BITS_PER_BONEREF) - 1);

    return boneIndex;
}

static ID_INLINE float G2_GetVertBoneWeight(const mdxmVertex_t *pVert, const int iWeightNum, float *fTotalWeight, int iNumWeights)
{
    float   fBoneWeight;
    int     iTemp;

    if(iWeightNum == iNumWeights - 1){
        fBoneWeight = 1.0f - *fTotalWeight;
    }else{
        iTemp = pVert->BoneWeightings[iWeightNum];
        iTemp |= (pVert->uiNmWeightsAndBoneIndexes >> (iG2_BONEWEIGHT_TOPBITS_SHIFT + (iWeightNum * 2))) & iG2_BONEWEIGHT_TOPBITS_AND;

        fBoneWeight = fG2_BONEWEIGHT_RECIPROCAL_MULT * iTemp;
        *fTotalWeight += fBoneWeight;
    }

    return fBoneWeight;
}

/*
=============================================
--------------------------------
Main collision detect functions.
--------------------------------
=============================================
*/

/*
==================
G2_DecideTraceLod

Checks if the specified LOD is available
in the Ghoul II file.

Returns the last LOD if the LOD exceeds
the numbers of LODs in the file, or the
same if it is valid.
==================
*/

static int G2_DecideTraceLod(CGhoul2Model_t *model, int useLod)
{
    int             returnLod;
    mdxmHeader_t    *mdxmHeader;

    // First, get the Ghoul II mesh file header.
    mdxmHeader = model->currentModel->modelData;

    // Are we overriding the LOD at the top level? If so, we can
    // afford to only check this level of the model.
    if(model->mLodBias != -1){
        returnLod = model->mLodBias;
    }else{
        returnLod = useLod;
    }

    // Ensure that we haven't selected a LOD
    // that doesn't exist for this model.
    if(returnLod >= mdxmHeader->numLODs){
        return mdxmHeader->numLODs - 1;
    }

    // LOD to use is valid.
    return returnLod;
}

/*
==================
G2_TransformEachSurface

Transforms all vertexes for the
given surface.
==================
*/

static void G2_TransformEachSurface(CGhoul2Model_t *model, const mdxmSurface_t *surface, vec3_t scale)
{
    int                     numVerts;
    int                     i, j, pos;
    int                     *piBoneReferences;
    float                   *transformedVerts;
    mdxaBone_t              *bone;
    mdxmVertex_t            *v;
    mdxmVertexTexCoord_t    *pTexCoords;
    vec3_t                  tempVert, tempNormal;
    int                     iNumWeights, iBoneIndex;
    float                   fTotalWeight, fBoneWeight;

    //
    // Deform the vertexes by the lerped bones.
    //
    piBoneReferences = (int *)((byte *)surface + surface->ofsBoneReferences);
    numVerts = surface->numVerts;

    // Allocate some space for the transformed verts to get put in.
    transformedVerts = Hunk_Alloc(numVerts * 5 * 4, h_low);
    model->mTransformedVertsArray[surface->thisSurfaceIndex] = (size_t)transformedVerts;

    //
    // Whip through and actually transform each vertex.
    //
    v = (mdxmVertex_t *)((byte *)surface + surface->ofsVerts);
    pTexCoords = (mdxmVertexTexCoord_t *)&v[numVerts];

    if ((scale[0] != 1.0) || (scale[1] != 1.0) || (scale[2] != 1.0)){
        for(i = 0; i < numVerts; i++){
            VectorClear(tempVert);
            VectorClear(tempNormal);

            iNumWeights = G2_GetVertWeights(v);
            fTotalWeight = 0.0f;
            for(j = 0; j < iNumWeights; j++){
                iBoneIndex = G2_GetVertBoneIndex(v, j);
                fBoneWeight = G2_GetVertBoneWeight(v, j, &fTotalWeight, iNumWeights);

                // Get bone and evaluate if necessary.
                bone = G2_BoneEval(model->mBoneCache, piBoneReferences[iBoneIndex]);

                tempVert[0] += fBoneWeight * (DotProduct(bone->matrix[0], v->vertCoords) + bone->matrix[0][3]);
                tempVert[1] += fBoneWeight * (DotProduct(bone->matrix[1], v->vertCoords) + bone->matrix[1][3]);
                tempVert[2] += fBoneWeight * (DotProduct(bone->matrix[2], v->vertCoords) + bone->matrix[2][3]);

                tempNormal[0] += fBoneWeight * DotProduct(bone->matrix[0], v->normal);
                tempNormal[1] += fBoneWeight * DotProduct(bone->matrix[1], v->normal);
                tempNormal[2] += fBoneWeight * DotProduct(bone->matrix[2], v->normal);
            }

            pos = i * 5;

            // Copy transformed verts into temporary space.
            transformedVerts[pos++] = tempVert[0] * scale[0];
            transformedVerts[pos++] = tempVert[1] * scale[1];
            transformedVerts[pos++] = tempVert[2] * scale[2];

            // We will need the S & T coordinates too for hit location
            // and hit material stuff.
            transformedVerts[pos++] = pTexCoords[i].texCoords[0];
            transformedVerts[pos]   = pTexCoords[i].texCoords[1];

            v++;
        }
    }else{
        pos = 0;

        for(i = 0; i < numVerts; i++){
            VectorClear(tempVert);
            VectorClear(tempNormal);

            iNumWeights = G2_GetVertWeights(v);
            fTotalWeight = 0.0f;

            for(j = 0; j < iNumWeights; j++){
                iBoneIndex = G2_GetVertBoneIndex(v, j);
                fBoneWeight = G2_GetVertBoneWeight(v, j, &fTotalWeight, iNumWeights);

                // Get bone and evaluate if necessary.
                bone = G2_BoneEval(model->mBoneCache, piBoneReferences[iBoneIndex]);

                tempVert[0] += fBoneWeight * (DotProduct(bone->matrix[0], v->vertCoords) + bone->matrix[0][3]);
                tempVert[1] += fBoneWeight * (DotProduct(bone->matrix[1], v->vertCoords) + bone->matrix[1][3]);
                tempVert[2] += fBoneWeight * (DotProduct(bone->matrix[2], v->vertCoords) + bone->matrix[2][3]);

                tempNormal[0] += fBoneWeight * DotProduct(bone->matrix[0], v->normal);
                tempNormal[1] += fBoneWeight * DotProduct(bone->matrix[1], v->normal);
                tempNormal[2] += fBoneWeight * DotProduct(bone->matrix[2], v->normal);
            }

            // Copy transformed verts into temporary space.
            transformedVerts[pos++] = tempVert[0];
            transformedVerts[pos++] = tempVert[1];
            transformedVerts[pos++] = tempVert[2];

            // We will need the S & T coordinates too for hit location
            // and hit material stuff.
            transformedVerts[pos++] = pTexCoords[i].texCoords[0];
            transformedVerts[pos++] = pTexCoords[i].texCoords[1];

            v++;
        }

    }
}

/*
==================
G2_TransformSurfaces_r

Recursively renders all surfaces
in the Ghoul II model.
==================
*/

static void G2_TransformSurfaces_r(CGhoul2Model_t *model, int surfaceIndex, int lod, vec3_t scale)
{
    mdxmSurface_t               *surface;
    mdxmHierarchyOffsets_t      *surfIndexes;
    mdxmSurfHierarchy_t         *surfInfo;
    int                         i, offFlags;

    // Back track and get the surface info structures for this surface.

    // Get surface.
    surface = G2_FindSurfaceFromModel(model->currentModel, surfaceIndex, lod);

    // Get surface index and hierarchy info.
    surfIndexes = (mdxmHierarchyOffsets_t *)((byte *)model->currentModel->modelData + sizeof(mdxmHeader_t));
    surfInfo = (mdxmSurfHierarchy_t *)((byte *)surfIndexes + surfIndexes->offsets[surface->thisSurfaceIndex]);

    // Get the flags for this surface so we can determine
    // whether it is on or off.
    offFlags = surfInfo->flags;

    // If this surface is not off, add it to the shader render list.
    if(!offFlags){
        G2_TransformEachSurface(model, surface, scale);
    }

    // Now recursively call for all the children.
    for(i = 0; i < surfInfo->numChildren; i++){
        G2_TransformSurfaces_r(model, surfInfo->childIndexes[i], lod, scale);
    }

}

/*
==============
G2_TransformModel

Main calling point for the model transform
for collision detection. At this point all
of the skeleton has been transformed.
==============
*/

void G2_TransformModel(CGhoul2Array_t *ghlInfo, const int frameNum, vec3_t scale, int useLod)
{
    CGhoul2Model_t  *model;
    mdxmHeader_t    *mdxmHeader;
    vec3_t          correctScale;
    int             i, numValid;
    int             lod;

    //
    // Check for scales of 0.
    //
    VectorCopy(scale, correctScale);
    for(i = 0; i < 3; i++){
        if(!scale[i]){
            correctScale[i] = 1.0f;
        }
    }

    //
    // Walk through each possible model for this entity
    // and try rendering it out.
    //
    numValid = 0;
    for(i = 0; i < G2_MAX_MODELS_IN_LIST; i++){
        model = ghlInfo->models[i];

        // Check if this slot is allocated.
        if(model != NULL){
            // Only work with valid models.
            if(model->mValid){
                // Get the Ghoul II mesh file header for this model.
                mdxmHeader = model->currentModel->modelData;

                // Decide the LOD.
                lod = G2_DecideTraceLod(model, useLod);

                // Give us space for the transformed vertex array to be put in.
                // If it is not allocated already that is.
                if(model->mTransformedVertsArray == NULL){
                    model->mTransformedVertsArray = Z_TagMalloc(mdxmHeader->numSurfaces * sizeof(size_t), TAG_GHOUL2);
                }
                memset(model->mTransformedVertsArray, 0, mdxmHeader->numSurfaces * sizeof(size_t));

                // Recursively transform the model surfaces.
                G2_TransformSurfaces_r(model, 0, lod, correctScale);
            }

            numValid++;
            if(numValid == ghlInfo->numModels){
                // Don't continue if we've iterated
                // through all allocated slots.
                break;
            }
        }
    }
}

/*
==============
G2_InitTraceSurf

Initializes a CTraceSurface_t structure
to trace against surfaces.
==============
*/

static void G2_InitTraceSurf(CTraceSurface_t *TS, CGhoul2Model_t *model, int lod, vec3_t rayStart, vec3_t rayEnd, mdxaBone_t *worldMatrix, CollisionRecord_t *collRecMap,
                             int entNum, skin_t *skin, int traceFlags)
{
    // Save info off our Ghoul II model.
    TS->currentModel = model->currentModel;
    TS->transformedVertsArray = model->mTransformedVertsArray;

    // Copy the rays.
    VectorCopy(rayStart, TS->rayStart);
    VectorCopy(rayEnd, TS->rayEnd);

    // Set remaining variables.
    TS->surfaceNum = 0;
    TS->lod = lod;
    TS->worldMatrix = worldMatrix;
    TS->collRecMap = collRecMap;
    TS->entNum = entNum;
    TS->skin = skin;
    TS->traceFlags = traceFlags;
    TS->stopRec = qfalse;
}

/*
==============
G2_SegmentTriangleTest

Function that works out given a ray
whether or not it hits a poly.
==============
*/

static qboolean G2_SegmentTriangleTest(vec3_t start, vec3_t end, vec3_t A, vec3_t B, vec3_t C,
                                       vec3_t returnedPoint, vec3_t returnedNormal, float *denom)
{
    vec3_t              returnedNormalT;
    vec3_t              edgeAC, edgePA, edgePB, edgePC;
    vec3_t              ray, toPlane;
    vec3_t              temp;
    float               t;
    static const float  tiny = 1E-10f;

    VectorSubtract(C, A, edgeAC);
    VectorSubtract(B, A, returnedNormalT);

    CrossProduct(returnedNormalT, edgeAC, returnedNormal);

    VectorSubtract(end, start, ray);
    *denom = DotProduct(ray, returnedNormal);

    if(fabs(*denom) < tiny){
        // Triangle parallel to ray.
        return qfalse;
    }

    VectorSubtract(A, start, toPlane);
    t = DotProduct(toPlane, returnedNormal) / *denom;

    if(t < 0.0f || t > 1.0f){
        // This is an off segment.
        return qfalse;
    }

    VectorMA(start, t, ray, returnedPoint);

    VectorSubtract(A, returnedPoint, edgePA);
    VectorSubtract(B, returnedPoint, edgePB);

    CrossProduct(edgePA, edgePB, temp);
    if(DotProduct(temp, returnedNormal) < 0.0f){
        // This is an off triangle.
        return qfalse;
    }

    VectorSubtract(C, returnedPoint, edgePC);
    CrossProduct(edgePC, edgePA, temp);
    if(DotProduct(temp,returnedNormal) < 0.0f){
        // This is an off triangle.
        return qfalse;
    }

    CrossProduct(edgePB, edgePC, temp);
    if(DotProduct(temp, returnedNormal) < 0.0f){
        // This is an off triangle.
        return qfalse;
    }

    // This poly is hit.
    return qtrue;
}

/*
==============
G2_AreaOfTri

Work out how much space
a triangle takes.
==============
*/

static float G2_AreaOfTri(const vec3_t A, const vec3_t B, const vec3_t C)
{
    vec3_t  cross, ab, cb;

    VectorSubtract(A, B, ab);
    VectorSubtract(C, B, cb);
    CrossProduct(ab, cb, cross);

    return VectorLength(cross);
}

/*
==============
G2_BuildHitPointST

Actually determine the S and T
of the coordinate we hit
in the given poly.
==============
*/

static void G2_BuildHitPointST(const vec3_t A, const float SA, const float TA,
                               const vec3_t B, const float SB, const float TB,
                               const vec3_t C, const float SC, const float TC,
                               const vec3_t P, float *s, float *t,
                               float *bary_i, float *bary_j)
{
    float   areaABC;
    float   i, j, k;

    areaABC = G2_AreaOfTri(A, B, C);
    i       = G2_AreaOfTri(P, B, C) / areaABC;
    j       = G2_AreaOfTri(A, P, C) / areaABC;
    k       = G2_AreaOfTri(A, B, P) / areaABC;

    *bary_i = i;
    *bary_j = j;

    *s      = SA * i + SB * j + SC * k;
    *t      = TA * i + TB * j + TC * k;

    *s      = fmod(*s, 1);
    if(*s < 0){
        *s += 1.0;
    }

    *t      = fmod(*t, 1);
    if(*t < 0){
        *t += 1.0;
    }
}

/*
==============
G2_TracePolys

Trace through the polys of a surface
to see what we've hit.
==============
*/

static qboolean G2_TracePolys(mdxmSurface_t *surface, mdxmSurfHierarchy_t *surfInfo, CTraceSurface_t *TS)
{
    mdxmTriangle_t      *tris;
    int                 i, j, x, numTris;
    float               face, xPos, yPos;
    float               *verts;
    float               *pointA, *pointB, *pointC;
    vec3_t              hitPoint, normal, distVect;
    CollisionRecord_t   *newCol;
    shader_t            *shader;
    hitRegData_t        *hitRegData;

    tris    = (mdxmTriangle_t *)((byte *)surface + surface->ofsTriangles);
    verts   = (float *)TS->transformedVertsArray[surface->thisSurfaceIndex];
    numTris = surface->numTriangles;

    // Iterate through the tris and
    // transform each vertex.
    for(i = 0; i < numTris; i++){
        // Determine the actual coordinates for this triangle.
        pointA = &verts[(tris[i].indexes[0] * 5)];
        pointB = &verts[(tris[i].indexes[1] * 5)];
        pointC = &verts[(tris[i].indexes[2] * 5)];

        // Did we hit it?
        if(G2_SegmentTriangleTest(TS->rayStart, TS->rayEnd, pointA, pointB, pointC, hitPoint, normal, &face)){
            // We did. Find space in the collision records for this record.
            for(x = 0; x < MAX_G2_COLLISIONS; x++){
                if(TS->collRecMap[x].mEntityNum != -1){
                    continue;
                }

                // Found a free one.
                newCol  = &TS->collRecMap[x];
                xPos    = 0;
                yPos    = 0;

                newCol->mPolyIndex = i;
                newCol->mEntityNum = TS->entNum;
                newCol->mSurfaceIndex = surface->thisSurfaceIndex;

                if(face > 0){
                    newCol->mFlags = G2_FRONTFACE;
                }else{
                    newCol->mFlags = G2_BACKFACE;
                }

                VectorSubtract(hitPoint, TS->rayStart, distVect);
                newCol->mDistance = VectorLength(distVect);

                // Put the hit point back into world space.
                G2_TransformTranslatePoint(hitPoint, newCol->mCollisionPosition, TS->worldMatrix);

                // Transform normal (but don't translate) into world angles.
                G2_TransformPoint(normal, newCol->mCollisionNormal, TS->worldMatrix);
                VectorNormalize(newCol->mCollisionNormal);

                newCol->mMaterial = newCol->mLocation = 0;

                // Determine our location within the texture
                // and the barycentric coordinates.
                G2_BuildHitPointST(pointA, pointA[3], pointA[4],
                                   pointB, pointB[3], pointB[4],
                                   pointC, pointC[3], pointC[4],
                                   hitPoint, &xPos, &yPos,
                                   &newCol->mBarycentricI,
                                   &newCol->mBarycentricJ);

                // Now we know what surface this hit belongs to,
                // we need to go get the correct hit location
                // and hit material.
                if(TS->skin){
                    shader = NULL;

                    // Match the the defined shader to a surface name in the skin surfaces.
                    for(j = 0; j < TS->skin->numSurfaces; j++){
                        if(strcmp(TS->skin->surfaces[j]->name, surfInfo->shader) == 0){
                            shader = TS->skin->surfaces[j]->shader;
                        }
                    }

                    // Do we have a valid shader file, and hit
                    // location and material info in it?
                    if(shader && (shader->hitLocation || (shader->hitMaterial))){
                        // We have a floating point position.
                        // Determine location in data we need to look at.
                        if(shader->hitLocation != -1){
                            hitRegData = &tr.hitRegData[shader->hitLocation];

                            newCol->mLocation = *(hitRegData->loc +
                                                ((int)(yPos * hitRegData->height) * hitRegData->width) +
                                                ((int)(xPos * hitRegData->width)));
                        }

                        if(shader->hitMaterial != -1){
                            hitRegData = &tr.hitRegData[shader->hitMaterial];

                            newCol->mMaterial = *(hitRegData->loc +
                                                ((int)(yPos * hitRegData->height) * hitRegData->width) +
                                                ((int)(xPos * hitRegData->width)));
                        }
                    }
                }

                // Exit now if we should.
                if(TS->traceFlags == G2_RETURNONHIT){
                    return qtrue;
                }

                break;
            }

            if(x == MAX_G2_COLLISIONS){
                // We've run out of collision record space.
                // Stop iterating.
                TS->stopRec = qtrue;
                return qtrue;
            }
        }
    }

    return qfalse;
}

/*
==============
G2_TraceSurfaces_r

Recursively trace the surfaces
to determine what we've hit.
==============
*/

static void G2_TraceSurfaces_r(CTraceSurface_t *TS)
{
    mdxmSurface_t               *surface;
    mdxmHierarchyOffsets_t      *surfIndexes;
    mdxmSurfHierarchy_t         *surfInfo;
    int                         i, offFlags;

    // Back track and get the surface info structures for this surface.

    // Get surface.
    surface = G2_FindSurfaceFromModel(TS->currentModel, TS->surfaceNum, TS->lod);

    // Get surface index and hierarchy info.
    surfIndexes = (mdxmHierarchyOffsets_t *)((byte *)TS->currentModel->modelData + sizeof(mdxmHeader_t));
    surfInfo = (mdxmSurfHierarchy_t *)((byte *)surfIndexes + surfIndexes->offsets[surface->thisSurfaceIndex]);

    // Get the flags for this surface so we can determine
    // whether it is on or off.
    offFlags = surfInfo->flags;

    // If this surface is not off, try to hit it.
    if(!offFlags){
        // Make sure we have (initialized) collision records.
        if(TS->collRecMap){
            // This is always a point trace.
            // So trace the polys in this surface.
            if(G2_TracePolys(surface, surfInfo, TS) && (TS->traceFlags == G2_RETURNONHIT)){
                // We hit one, and we want to return instantly
                // because the G2_RETURNONHIT flag is set.
                TS->stopRec = qtrue;
                return;
            }
        }
    }

    // If we are turning off all descendants,
    // then stop this recursion now.
    if(offFlags & G2SURFACEFLAG_NODESCENDANTS){
        return;
    }

    // Recursively call for all children.
    for(i = 0; i < surfInfo->numChildren && !TS->stopRec; i++){
        TS->surfaceNum = surfInfo->childIndexes[i];
        G2_TraceSurfaces_r(TS);
    }
}

/*
==============
G2_TraceModels

Trace against all Ghoul II models
in the given Ghoul II array.
==============
*/

void G2_TraceModels(CGhoul2Array_t *ghlInfo, vec3_t rayStart, vec3_t rayEnd, mdxaBone_t *worldMatrix, CollisionRecord_t *collRecMap, int entNum, int traceFlags, int useLod)
{
    CGhoul2Model_t  *model;
    skin_t          *skin;
    CTraceSurface_t TS;
    int             i;
    int             numValid;
    int             lod;

    //
    // Walk each possible model for this entity
    // and try tracing against it.
    //
    numValid = 0;

    for(i = 0; i < G2_MAX_MODELS_IN_LIST; i++){
        model = ghlInfo->models[i];

        // Check if this slot is allocated.
        if(model != NULL){
            // Only work with valid models.
            if(model->mValid){
                // Decide the LOD.
                lod = G2_DecideTraceLod(model, useLod);

                // Is a custom skin set to be used?
                if(model->mCustomSkin != -1 && model->mCustomSkin < tr.numSkins){
                    skin = R_GetSkinByHandle(model->mCustomSkin);
                }else{
                    skin = NULL;
                }

                // Initialize our trace surface structure.
                G2_InitTraceSurf(&TS, model, lod, rayStart, rayEnd, worldMatrix, collRecMap, entNum, skin, traceFlags);

                // Start the surface recursion loop.
                G2_TraceSurfaces_r(&TS);
            }

            numValid++;
            if(numValid == ghlInfo->numModels){
                // Don't continue if we've iterated
                // through all allocated slots.
                break;
            }
        }
    }
}
