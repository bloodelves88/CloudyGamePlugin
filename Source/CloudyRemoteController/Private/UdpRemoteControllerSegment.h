#pragma once

namespace EUdpRemoteControllerSegment
{
	enum Type
	{
		None = 0,
		KeyboardInput = 1,
		MouseInput = 2,
		Data = 3
	};
}

namespace FUdpRemoteControllerSegment {
	struct FHeaderChunk
	{
		uint8 Version;

		/**
		 * Segment Type
		 * @see EUdpRemoteControllerSegment
		 */
		uint8 SegmentType;

		friend FArchive& operator<<(FArchive& Ar, FHeaderChunk& Chunk)
		{
			return Ar
				<< Chunk.Version
				<< Chunk.SegmentType;
		}
	};

	struct FKeyboardInputChunk
	{
		uint8 ControllerID;
		int16 KeyCode;
		int16 CharCode;
		uint8 InputEvent;

		friend FArchive& operator<<(FArchive& Ar, FKeyboardInputChunk& Chunk)
		{
			return Ar
				<< Chunk.ControllerID
				<< Chunk.KeyCode
				<< Chunk.CharCode
				<< Chunk.InputEvent;
		}
	};

	struct FMouseInputChunk
	{
		uint8 ControllerID;
		int16 XAxis;
		int16 YAxis;
		float XPos;
		float YPos;

		friend FArchive& operator<<(FArchive& Ar, FMouseInputChunk& Chunk)
		{
			return Ar
				<< Chunk.ControllerID
				<< Chunk.XAxis
				<< Chunk.YAxis
				<< Chunk.XPos
				<< Chunk.YPos;
		}
	};
}