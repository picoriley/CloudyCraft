#pragma once
#include "Game/GameCommon.hpp"
#include "Game/BlockDefinition.h"

class Face;

class Block
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Block();
	Block(unsigned char type);

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaTime);
	void Render() const;
	inline bool IsSky() { return (m_lightAndFlags & SKY_BIT) != 0; }
	inline bool IsEdgeBlock() { return(m_lightAndFlags & EDGE_BIT) != 0; }
	inline bool IsDirty() { return (m_lightAndFlags & LIGHTING_DIRTY_BIT) != 0; }
	inline bool IsAboveVisible() { return (m_hsrFlags & ABOVE_BIT) != 0; }
	inline bool IsBelowVisible() { return (m_hsrFlags & BELOW_BIT) != 0; }
	inline bool IsNorthVisible() { return (m_hsrFlags & NORTH_BIT) != 0; }
	inline bool IsSouthVisible() { return (m_hsrFlags & SOUTH_BIT) != 0; }
	inline bool IsEastVisible() { return (m_hsrFlags & EAST_BIT) != 0; }
	inline bool IsWestVisible() { return (m_hsrFlags & WEST_BIT) != 0; }

	//GETTERS//////////////////////////////////////////////////////////////////////////
	inline BlockDefinition* GetDefinition() { return BlockDefinition::GetDefinition(m_type); }
	inline unsigned int GetLightValue() const;
	inline unsigned int GetDampedLightValue(uchar dampAmount) const;
	inline RGBA GetRGBALightValue() const;
	inline uchar GetRedLightValue() const;
	inline uchar GetGreenLightValue() const;
	inline uchar GetBlueLightValue() const;
	static Face GetFace(const Vector3Int& bottomLeftCorner, const Vector3Int& faceNormal);

	//SETTERS//////////////////////////////////////////////////////////////////////////
	inline void SetLightValue(const RGBA& lightColor);
	inline void SetRedLightValue(uchar lightValue0To15);
	inline void SetGreenLightValue(uchar lightValue0To15);
	inline void SetBlueLightValue(uchar lightValue0To15);
	inline void SetSky(bool isSkyBlock);
	inline void SetEdgeBlock(bool isEdgeBlock);
	inline void SetDirty(bool isLightingDirty);
	inline void SetBelowVisible(bool isVisible);
	inline void SetAboveVisible(bool isVisible);
	inline void SetNorthVisible(bool isVisible);
	inline void SetSouthVisible(bool isVisible);
	inline void SetEastVisible(bool isVisible);
	inline void SetWestVisible(bool isVisible);
	void SetVisible(Direction visibleFace);
	void SetHidden(Direction hiddenFace);

	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const uchar BITMASK_BLOCK_LIGHT = 0x0F; // 00001111 (bits 0-3) store light value, 0 to 15
	static const uchar BITMASK_BLOCK_IS_SKY = 0x80; // 10000000 (bit 7) is on if block sees sky
	static const uchar BITMASK_BLOCK_LIGHT_DIRTY = 0x40; // 01000000 (bit 6) is on if light is dirty
	static const uchar BITMASK_BLOCK_IS_EDGE = 0x20; // 00100000 (bit 5) on if block type is an edge block
	static const uchar BITMASK_ABOVE_VISIBLE = 0x20; // 00100000 (bit 5) on if the above face is visible
	static const uchar BITMASK_BELOW_VISIBLE = 0x10; // 00010000 (bit 4) on if the below face is visible
	static const uchar BITMASK_NORTH_VISIBLE = 0x08; // 00001000 (bit 3) on if the north face is visible
	static const uchar BITMASK_SOUTH_VISIBLE = 0x04; // 00000100 (bit 2) on if the south face is visible
	static const uchar BITMASK_EAST_VISIBLE = 0x02;  // 00000010 (bit 1) on if the east face is visible
	static const uchar BITMASK_WEST_VISIBLE = 0x01;  // 00000001 (bit 0) on if the west face is visible

	static const uchar ABOVE_BIT          = BIT(5);
	static const uchar BELOW_BIT          = BIT(4);
	static const uchar NORTH_BIT          = BIT(3);
	static const uchar SOUTH_BIT          = BIT(2);
	static const uchar EAST_BIT           = BIT(1);
	static const uchar WEST_BIT           = BIT(0);
	static const uchar SKY_BIT            = BIT(7);
	static const uchar LIGHTING_DIRTY_BIT = BIT(6);
	static const uchar EDGE_BIT           = BIT(5);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	uchar m_type;
	uchar m_lightAndFlags;
	uchar m_redLight;
	uchar m_greenLight;
	uchar m_blueLight;
	uchar m_hsrFlags;
}; 

#include "Game\Block.inl"
