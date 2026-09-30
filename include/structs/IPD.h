#pragma once
#include <cstdint>
#include "Shared.h"

#pragma pack(push, 1)

// Adapted from binary template (sh1_model.bt) by Sparagas (https://github.com/Sparagas/Silent-Hill)

// ~~~~~~~~~~~~ COLLISION ~~~~~~~~~~~~
// IPD_COLL_SURFACE (size: 12 bytes)
struct IPD_COLL_SURFACE {
    int16_t         relativeX;                      // Relative X (Q7.8)
    int16_t         baseGroundHeight;
    int16_t         relativeZ;                      // Relative Z (Q7.8)
    uint16_t        groundType          : 5;        // related to enum e_GroundType in engine (concrete, grass, etc)
    uint16_t        disableHeight       : 3;        // bool(s?)
    uint16_t        field_6_8           : 3;        // related to special collision triggers (s_CollisionState)
    uint16_t        field_6_11          : 4;        // surface slope?
    uint16_t        __pad               : 1;
    int16_t         tiltAngleX;                     // Slope gradient (dY/dX, Q7.8)
    int16_t         tiltAngleZ;                     // Slope gradient (dY/dZ, Q7.8)
};

// IPD_COLL_SUBCELL (size: 10 bytes)
//      2D line segments that are floor-material boundaries OR solid physical walls 
struct IPD_COLL_SUBCELL {
    int16_t         collLineX           : 14;       // 2D line transform, loaded into GTE R11
    uint16_t        collTriggerUpper    : 2;        // Collision trigger upper section
    int16_t         collLineZ           : 14;       // 2D line transform, loaded into GTE R12
    uint16_t        collTriggerLower    : 2;        // Collision trigger lower section
    int16_t         collLineLength;
    uint8_t         splitVertexIdx0;
    uint8_t         splitVertexIdx1;
    uint8_t         surfaceIdx0;
    uint8_t         surfaceIdx1;
};

// IPD_COLL_SUBCELL_RANGE (size: 4 bytes)
struct IPD_COLL_SUBCELL_RANGE {
    int16_t         offset_wallCylinder_indices_start;
    int16_t         offset_floorSurface_indices_start;
};

// IPD_COLL_CYLINDER (size: 10 bytes)
struct IPD_COLL_CYLINDER {
    uint16_t        groundType          : 5;        // `e_GroundType` of the obstacle
    uint16_t        disableHeight       : 3;        // bool -- whether the top surface can be stood on
                                                    //      false = standard obstacle (crate, curb)
                                                    //      true  = infinitely tall obstacle (tree)
    uint16_t        eventTrigger        : 4;        // dynamic enabling/disabling of obstacle
    uint16_t        unknown             : 3;        // Set as '0' across all IPD files
                                                    // Hypothesis: cylinder interaction type (search/inspect, vault) but unused in game/decomp?
    uint16_t        __pad               : 1;
    SVECTOR3        offset;                         // (vx, vz) -- position on map plane (relative to its center)
                                                    // vy       -- top surface/elevation
    uint16_t        radius;                         // r        -- horizontal radius of cylinder
};

struct IPD_COLL_SPLIT_VERTEX {
    uint8_t         placeholder;                    // Unknown and not mapped out properly
                                                    // May need to pluralise
};

// IPD_COLL_HEADER (size: 308 bytes)
struct IPD_COLL_HEADER {
    int32_t         positionX;                      // Chunk world X (Q23.8)
    int32_t         positionZ;                      // Chunk world Z (Q23.8)
    uint8_t         count_IPD_COLL_SPLIT_VERTEX;    // Missing
    uint8_t         count_IPD_COLL_SURFACE;
    uint8_t         count_IPD_COLL_SUBCELL;
    uint8_t         count_IPD_COLL_CYLINDER;
    uint32_t        offset_IPD_COLL_SPLIT_VERTEX;
    uint32_t        offset_IPD_COLL_SURFACE;
    uint32_t        offset_IPD_COLL_SUBCELL;        // Adjusted from Sparagas, offset_14 -> offset_subcells
                                                    // Wall/obstacle elements, IpdColl_TestWallElement.c (verify)
    uint32_t        offset_IPD_COLL_CYLINDER;       // Adjusted from Sparagas, offset_18 -> offset_cylinderColliders
                                                    // Cylindrical colliders, IpdColl_TestFloorElement.c (street poles, trees, verify)
    int16_t         subcellSize;                    // This and everything below needs to be verified and improved!
    int8_t          count_subcellX;
    int8_t          count_subcellZ;
    uint32_t        offset_subcellRanges;
    uint16_t        count_offset_wallCylinder_indices;  // Clarify the wording on these next 4 members
    uint16_t        count_offset_floorSurface_indices;
    uint32_t        offset_wallCylinder_indices;
    uint32_t        offset_floorSurface_indices;
    uint8_t         count_subcellCheck;
    uint8_t         __pad[3];
    uint8_t         subcellCheckIdx[256];
};

// IPD_MODEL_INSTANCE (size: 36 bytes)
struct IPD_MODEL_INSTANCE {
    uint32_t        offset_modelHdr;                   // connects to LM_MODEL_HEADER
    MATRIX          mat;
};

// IPD_MODEL_INFO (size: 16 bytes)
struct IPD_MODEL_INFO {
    uint8_t         isGlobalPlm;                    // (0) inside IPD, (1) from `*_GLB.PLM`
    int8_t          __pad[3];
    u_Filename      name;							// Asset name within the PLM
													// 		based purely on prefix? (verify)
    uint32_t        offset_modelHdr;
};

// IPD_MODEL_BUFFER (size: 24 bytes)
struct IPD_MODEL_BUFFER {
    uint8_t         count_modelInstance;             // modelInstances count
    uint8_t         count_billboardInstance;         // Unknown in Sparagas: field_1 -> billboardCount
                                                    //      Streetlamp glows, light source flares
    uint8_t         count_subcell;
    int8_t          __pad;
    int16_t         minX;
    int16_t         maxX;
    int16_t         minZ;
    int16_t         maxZ;
    uint32_t        offset_modelInstances;
    uint32_t        offset_billboardInstances;         // Unknown in Sparagas: offset_field_10 -> offset_billboardInstances
    uint32_t        offset_subcellPositions;
};

// IPD_SUBCELL_RANGE (size: 2 bytes)
struct IPD_SUBCELL_RANGE {
    uint8_t         startIndex;
    uint8_t         count;
};

// IPD_SUBCELL_VISIBILITY_TABLE (size: 52 bytes)
struct IPD_SUBCELL_VISIBILITY_TABLE {
    IPD_SUBCELL_RANGE   subcells[5][5];             // 25 subcells
    uint8_t         __pad[2];
};

// IPD_SUBCELL_AABB (size: 8 bytes)
struct IPD_SUBCELL_AABB {
    int16_t         minX;
    int16_t         maxX;
    int16_t         minZ;
    int16_t         maxZ;
};

// IPD_BILLBOARD_INSTANCE (size: 8 bytes)
struct IPD_BILLBOARD_INSTANCE {
    SVECTOR3        coords;
    int8_t          type;                           // 0 = lamp/glow, 1 = flare
    int8_t          __pad;
};

// IPD_HEADER (size: 84 bytes w/o collision)
struct IPD_HEADER {
    uint8_t         id;
    uint8_t         isLoaded;
    int8_t          cellX;
    int8_t          cellZ;
    uint32_t        offset_LM_HEADER;					// Internally embedded LMs
    uint8_t         count_LM_MODEL_HEADER;
    uint8_t         modelBufferCount;
    uint8_t         modelOrderCount;
    uint8_t         __pad[9];
    uint32_t        offset_modelInfos;
    uint32_t        offset_modelBuffers;
    IPD_SUBCELL_VISIBILITY_TABLE    visibilityTable;
    uint32_t        offset_LM_MODEL_ORDER;          // Missing from sh1-level-editor

    bool isValid() const {
        return (id == 0x14);
    }
};

#pragma pack(pop)