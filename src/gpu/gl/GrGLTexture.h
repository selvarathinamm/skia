/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "GrGpu.h"
#include "GrTexture.h"
#include "GrGLUtil.h"

class GrGLGpu;

class GrGLTexture : public GrTexture {
public:
    // Texture state that overlaps with sampler object state. We don't need to track this if we
    // are using sampler objects.
    struct SamplerParams {
        // These are the OpenGL defaults.
        GrGLenum fMinFilter = GR_GL_NEAREST_MIPMAP_LINEAR;
        GrGLenum fMagFilter = GR_GL_LINEAR;
        GrGLenum fWrapS = GR_GL_REPEAT;
        GrGLenum fWrapT = GR_GL_REPEAT;
        GrGLfloat fMinLOD = -1000.f;
        GrGLfloat fMaxLOD = 1000.f;
        void invalidate() {
            fMinFilter = ~0U;
            fMagFilter = ~0U;
            fWrapS = ~0U;
            fWrapT = ~0U;
            fMinLOD = SK_ScalarNaN;
            fMaxLOD = SK_ScalarNaN;
        }
    };

    // Texture state that does not overlap with sampler object state.
    struct NonSamplerParams {
        // These are the OpenGL defaults.
        uint32_t fSwizzleKey = GrSwizzle::RGBA().asKey();
        GrGLint fBaseMipMapLevel = 0;
        GrGLint fMaxMipMapLevel = 1000;
        void invalidate() {
            fSwizzleKey = ~0U;
            fBaseMipMapLevel = ~0;
            fMaxMipMapLevel = ~0;
        }
    };

    struct IDDesc {
        GrGLTextureInfo             fInfo;
        GrBackendObjectOwnership    fOwnership;
    };

    static GrTextureType TextureTypeFromTarget(GrGLenum textureTarget);

    GrGLTexture(GrGLGpu*, SkBudgeted, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    ~GrGLTexture() override {
        // check that invokeReleaseProc has been called (if needed)
        SkASSERT(!fReleaseHelper);
    }

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {
        fSamplerParams.invalidate();
        fNonSamplerParams.invalidate();
    }

    void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {
        fReleaseHelper = std::move(releaseHelper);
    }

    // These functions are used to track the texture parameters associated with the texture.
    GrGpu::ResetTimestamp getCachedParamsTimestamp() const { return fParamsTimestamp; }
    const SamplerParams& getCachedSamplerParams() const { return fSamplerParams; }
    const NonSamplerParams& getCachedNonSamplerParams() const { return fNonSamplerParams; }

    void setCachedParams(const SamplerParams* samplerParams,
                         const NonSamplerParams& nonSamplerParams,
                         GrGpu::ResetTimestamp currTimestamp) {
        if (samplerParams) {
            fSamplerParams = *samplerParams;
        }
        fNonSamplerParams = nonSamplerParams;
        fParamsTimestamp = currTimestamp;
    }

    GrGLuint textureID() const { return fID; }

    GrGLenum target() const;

    bool hasBaseLevelBeenBoundToFBO() const { return fBaseLevelHasBeenBoundToFBO; }
    void baseLevelWasBoundToFBO() { fBaseLevelHasBeenBoundToFBO = true; }

    static sk_sp<GrGLTexture> MakeWrapped(GrGLGpu*, const GrSurfaceDesc&, GrMipMapsStatus,
                                          const IDDesc&, bool purgeImmediately);

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    enum Wrapped { kWrapped };
    // Constructor for instances wrapping backend objects.
    GrGLTexture(GrGLGpu*, Wrapped, const GrSurfaceDesc&, GrMipMapsStatus, const IDDesc&,
                bool purgeImmediately);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override;

private:
    void invokeReleaseProc() {
        if (fReleaseHelper) {
            // Depending on the ref count of fReleaseHelper this may or may not actually trigger the
            // ReleaseProc to be called.
            fReleaseHelper.reset();
        }
    }

    SamplerParams fSamplerParams;
    NonSamplerParams fNonSamplerParams;
    GrGpu::ResetTimestamp fParamsTimestamp;
    sk_sp<GrReleaseProcHelper> fReleaseHelper;
    GrGLuint fID;
    GrGLenum fFormat;
    GrBackendObjectOwnership fTextureIDOwnership;
    bool fBaseLevelHasBeenBoundToFBO = false;

    typedef GrTexture INHERITED;
};

#endif
