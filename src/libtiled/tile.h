/*
 * tile.h
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "object.h"
#include "tiled.h"

#include <QPixmap>
#include <QSharedPointer>
#include <QUrl>

#include <memory>

namespace Tiled {

class ObjectGroup;
class Terrain;
class Tileset;

/**
 * Convenience function for creating tile terrain information.
 */
inline unsigned makeTerrain(int id)
{
    id &= 0xFF;
    return id << 24 | id << 16 | id << 8 | id;
}

/**
 * Convenience function for creating tile terrain information.
 */
inline unsigned makeTerrain(int topLeft,
                            int topRight,
                            int bottomLeft,
                            int bottomRight)
{
    return (topLeft & 0xFF) << 24 |
           (topRight & 0xFF) << 16 |
           (bottomLeft & 0xFF) << 8 |
           (bottomRight & 0xFF);
}

/**
 * Returns the given \a terrain with the \a corner modified to \a terrainId.
 */
inline unsigned setTerrainCorner(unsigned terrain, int corner, int terrainId)
{
    unsigned mask = 0xFF << (3 - corner) * 8;
    unsigned insert = terrainId << (3 - corner) * 8;
    return (terrain & ~mask) | (insert & mask);
}

/**
 * A single frame of an animated tile.
 */
struct Frame
{
    bool operator == (const Frame &frame) const
    {
        return tileId == frame.tileId &&
                duration == frame.duration;
    }

    int tileId;
    int duration;
};

class TILEDSHARED_EXPORT Tile : public Object
{
    Q_OBJECT

public:
    Tile(int id, Tileset *tileset);
    Tile(const QPixmap &image, int id, Tileset *tileset);

    ~Tile();

    int id() const;

    Tileset *tileset() const;
    QSharedPointer<Tileset> sharedTileset() const;

    const QPixmap &image() const;
    void setImage(const QPixmap &image);

    const Tile *currentFrameTile() const;

    const QUrl &imageSource() const;
    void setImageSource(const QUrl &imageSource);

    int width() const;
    int height() const;
    QSize size() const;

    QPoint offset() const;

    const QString &type() const;
    void setType(const QString &type);

    Terrain *terrainAtCorner(int corner) const;

    int cornerTerrainId(int corner) const;
    void setCornerTerrainId(int corner, int terrainId);

    inline unsigned terrain() const;
    void setTerrain(unsigned terrain);

    qreal probability() const;
    void setProbability(qreal probability);

    ObjectGroup *objectGroup() const;
    void setObjectGroup(std::unique_ptr<ObjectGroup> &&objectGroup);
    void swapObjectGroup(std::unique_ptr<ObjectGroup> &objectGroup);

    const QVector<Frame> &frames() const;
    void setFrames(const QVector<Frame> &frames);
    bool isAnimated() const;
    int currentFrameIndex() const;
    bool resetAnimation();
    bool advanceAnimation(int ms);

    LoadingStatus imageStatus() const;
    void setImageStatus(LoadingStatus status);

    Tile *clone(Tileset *tileset) const;

private:
    int mId;
    Tileset *mTileset;
    QPixmap mImage;
    QUrl mImageSource;
    LoadingStatus mImageStatus;
    QString mType;
    unsigned mTerrain;
    qreal mProbability;
    std::unique_ptr<ObjectGroup> mObjectGroup;

    QVector<Frame> mFrames;
    int mCurrentFrameIndex;
    int mUnusedTime;

    friend class Tileset; // To allow changing the tile id
};

/**
 * Returns ID of this tile within its tileset.
 */
inline int Tile::id() const
{
    return mId;
}

/**
 * Returns the tileset that this tile is part of.
 */
inline Tileset *Tile::tileset() const
{
    return mTileset;
}

/**
 * Returns the image of this tile.
 */
inline const QPixmap &Tile::image() const
{
    return mImage;
}

/**
 * Sets the image of this tile.
 */
inline void Tile::setImage(const QPixmap &image)
{
    mImage = image;
    mImageStatus = image.isNull() ? LoadingError : LoadingReady;
}

/**
 * Returns the URL of the external image that represents this tile.
 * When this tile doesn't refer to an external image, an empty URL is
 * returned.
 */
inline const QUrl &Tile::imageSource() const
{
    return mImageSource;
}

inline void Tile::setImageSource(const QUrl &imageSource)
{
    mImageSource = imageSource;
}

/**
 * Returns the width of this tile.
 */
inline int Tile::width() const
{
    return mImage.width();
}

/**
 * Returns the height of this tile.
 */
inline int Tile::height() const
{
    return mImage.height();
}

/**
 * Returns the size of this tile.
 */
inline QSize Tile::size() const
{
    return mImage.size();
}

/**
 * Returns the type of this tile. Tile objects that do not have a type
 * explicitly set on them are assumed to be of the type returned by this
 * function.
 */
inline const QString &Tile::type() const
{
    return mType;
}

/**
 * Sets the type of this tile.
 * \sa type()
 */
inline void Tile::setType(const QString &type)
{
    mType = type;
}

/**
 * Returns the terrain id at a given corner.
 */
inline int Tile::cornerTerrainId(int corner) const
{
    unsigned t = (terrain() >> (3 - corner)*8) & 0xFF; return t == 0xFF ? -1 : (int)t;
}

/**
 * Set the terrain type of a given corner.
 */
inline void Tile::setCornerTerrainId(int corner, int terrainId)
{
    setTerrain(setTerrainCorner(mTerrain, corner, terrainId));
}

/**
 * Returns the terrain for each corner of this tile.
 */
inline unsigned Tile::terrain() const
{
    return mTerrain;
}

/**
 * Returns the relative probability of this tile appearing while painting.
 */
inline qreal Tile::probability() const
{
    return mProbability;
}

/**
 * Set the relative probability of this tile appearing while painting.
 */
inline void Tile::setProbability(qreal probability)
{
    mProbability = probability;
}

/**
 * @return The group of objects associated with this tile. This is generally
 *         expected to be used for editing collision shapes.
 */
inline ObjectGroup *Tile::objectGroup() const
{
    return mObjectGroup.get();
}

inline const QVector<Frame> &Tile::frames() const
{
    return mFrames;
}

inline bool Tile::isAnimated() const
{
    return !mFrames.isEmpty();
}

inline int Tile::currentFrameIndex() const
{
    return mCurrentFrameIndex;
}

/**
 * Returns the loading status of the image referenced by this tile.
 */
inline LoadingStatus Tile::imageStatus() const
{
    return mImageStatus;
}

inline void Tile::setImageStatus(LoadingStatus status)
{
    mImageStatus = status;
}

} // namespace Tiled
