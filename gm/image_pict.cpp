/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkImageCacherator.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "../src/image/SkImage_Gpu.h"
#endif

static void draw_something(SkCanvas* canvas, const SkRect& bounds) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    canvas->drawRect(bounds, paint);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SK_ColorBLUE);
    canvas->drawOval(bounds, paint);
}

/*
 *  Exercise drawing pictures inside an image, showing that the image version is pixelated
 *  (correctly) when it is inside an image.
 */
class ImagePictGM : public skiagm::GM {
    SkAutoTUnref<SkPicture> fPicture;
    SkAutoTUnref<SkImage>   fImage0;
    SkAutoTUnref<SkImage>   fImage1;
public:
    ImagePictGM() {}

protected:
    SkString onShortName() override {
        return SkString("image-picture");
    }

    SkISize onISize() override {
        return SkISize::Make(850, 450);
    }

    void onOnceBeforeDraw() override {
        const SkRect bounds = SkRect::MakeXYWH(100, 100, 100, 100);
        SkPictureRecorder recorder;
        draw_something(recorder.beginRecording(bounds), bounds);
        fPicture.reset(recorder.endRecording());

        // extract enough just for the oval.
        const SkISize size = SkISize::Make(100, 100);

        SkMatrix matrix;
        matrix.setTranslate(-100, -100);
        fImage0.reset(SkImage::NewFromPicture(fPicture, size, &matrix, nullptr));
        matrix.postTranslate(-50, -50);
        matrix.postRotate(45);
        matrix.postTranslate(50, 50);
        fImage1.reset(SkImage::NewFromPicture(fPicture, size, &matrix, nullptr));
    }

    void drawSet(SkCanvas* canvas) const {
        SkMatrix matrix = SkMatrix::MakeTrans(-100, -100);
        canvas->drawPicture(fPicture, &matrix, nullptr);
        canvas->drawImage(fImage0, 150, 0);
        canvas->drawImage(fImage1, 300, 0);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20, 20);

        this->drawSet(canvas);

        canvas->save();
        canvas->translate(0, 130);
        canvas->scale(0.25f, 0.25f);
        this->drawSet(canvas);
        canvas->restore();

        canvas->save();
        canvas->translate(0, 200);
        canvas->scale(2, 2);
        this->drawSet(canvas);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ImagePictGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////


class ImageCacheratorGM : public skiagm::GM {
    SkAutoTUnref<SkPicture>         fPicture;
    SkAutoTDelete<SkImageCacherator> fCache;

public:
    ImageCacheratorGM() {}

protected:
    SkString onShortName() override {
        return SkString("image-cacherator");
    }

    SkISize onISize() override {
        return SkISize::Make(850, 450);
    }

    void onOnceBeforeDraw() override {
        const SkRect bounds = SkRect::MakeXYWH(100, 100, 100, 100);
        SkPictureRecorder recorder;
        draw_something(recorder.beginRecording(bounds), bounds);
        fPicture.reset(recorder.endRecording());

        // extract enough just for the oval.
        const SkISize size = SkISize::Make(100, 100);

        SkMatrix matrix;
        matrix.setTranslate(-100, -100);
        auto gen = SkImageGenerator::NewFromPicture(size, fPicture, &matrix, nullptr);
        fCache.reset(SkImageCacherator::NewFromGenerator(gen));
    }

    void drawSet(SkCanvas* canvas) const {
        SkMatrix matrix = SkMatrix::MakeTrans(-100, -100);
        canvas->drawPicture(fPicture, &matrix, nullptr);

        {
            SkBitmap bitmap;
            fCache->lockAsBitmap(&bitmap);
            canvas->drawBitmap(bitmap, 150, 0);
        }
#if SK_SUPPORT_GPU
        {
            SkAutoTUnref<GrTexture> texture(fCache->lockAsTexture(canvas->getGrContext(),
                                                                   kUntiled_SkImageUsageType));
            if (!texture) {
                return;
            }
            // No API to draw a GrTexture directly, so we cheat and create a private image subclass
            SkAutoTUnref<SkImage> image(new SkImage_Gpu(100, 100, fCache->generator()->uniqueID(),
                                                        kPremul_SkAlphaType, texture, 0,
                                                        SkSurface::kNo_Budgeted));
            canvas->drawImage(image, 300, 0);
        }
#endif
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20, 20);

        this->drawSet(canvas);

        canvas->save();
        canvas->translate(0, 130);
        canvas->scale(0.25f, 0.25f);
        this->drawSet(canvas);
        canvas->restore();

        canvas->save();
        canvas->translate(0, 200);
        canvas->scale(2, 2);
        this->drawSet(canvas);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ImageCacheratorGM; )



