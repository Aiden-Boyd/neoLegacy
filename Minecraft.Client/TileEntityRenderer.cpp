#include "stdafx.h"
#include "TileEntityRenderer.h"
#include "TileEntityRenderDispatcher.h"

void TileEntityRenderer::bindTexture(ResourceLocation *location)
{
    Textures *t = tileEntityRenderDispatcher->textures;
    if(t != nullptr && location != nullptr) t->bindTexture(location);
}

void TileEntityRenderer::bindTexture(const wstring& urlTexture, ResourceLocation *location)
{
    Textures *t = tileEntityRenderDispatcher->textures;
    if(t != nullptr && location != nullptr) {
        int texture=location->isPreloaded()?t->loadHttpTexture(urlTexture,location->getTexture()):t->loadHttpTexture(urlTexture,location->getPath());
        if(texture>=0)t->bind(texture);
    }
}

int TileEntityRenderer::getHeight(ResourceLocation *location)
{
    Textures *t = tileEntityRenderDispatcher->textures;
    if(t != nullptr && location != nullptr) {
        // Build a temporary, not a shallow copy of ResourceLocation's owned texture array.
        if(location->isPreloaded())return t->getHeight(location->getTexture());
        return t->getHeight(ResourceLocation(location->getPath()));
    }
    return 32;
}

Level *TileEntityRenderer::getLevel()
{
	return tileEntityRenderDispatcher->level;
}

void TileEntityRenderer::init(TileEntityRenderDispatcher *tileEntityRenderDispatcher)
{
	this->tileEntityRenderDispatcher = tileEntityRenderDispatcher;
}

Font *TileEntityRenderer::getFont()
{
	return tileEntityRenderDispatcher->getFont();
}
