#pragma once

#include "spargel/base/string_view.h"

namespace spargel::text {

    // A fallback option for font families.
    //
    // These are CSS values.
    enum class GenericFontFamily {
        serif,
        sans_serif,
        monospace,
        cursive,
        fantasy,
        system_ui,
        ui_serif,
        ui_sans_serif,
        ui_monospace,
        ui_rounded,
        emoji,
        math,
        fangsong,
    };

    // A font family list consists of a list of font families ordered by priority from the highest
    // to the lowest. Font selection happens one character at a time. (TODO: what is a character)
    // The list includes a (predefined) generic font family at the end.
    struct FontFamilyList {
        // The first family has the highest priority.
        base::Vector<base::StringView> family_names;
        // A fallback family. It must be provided.
        GenericFontFamily generic_family;
    };

    // Selecting a font with the specific style within a font family.
    //
    // NOTE:
    // It seems that style is encoded as "Font Subfmaily" (also called "Style" in Apple's spec) in
    // the `name` table of TrueType/OpenType fonts. However the subfamily is language-dependent
    // text.
    //
    // NOTE:
    // In the CSS model, oblique has an optional argument specifying the angle.
    enum class FontStyle {
        // The normal font in a font family.
        normal,
        // Upright and lack of slant.
        //
        // This value is not present in CSS, but in Pango.
        roman,
        // Oblique (also called slant) is sloped normal.
        // Sometimes it can be simulated by applying an affine transformation to the normal style.
        //
        // If oblique is not available, fallback to italic.
        // If neither is available, synthesize.
        oblique,
        // If italic is not available, fallback to oblique.
        // If neither is available, synthesize.
        italic,
    };

    enum class FontVariant {
        small_caps,
        // TODO: More variants.
    };

    // A dictionary of attributes that is used to determine a font.
    //
    // Font descriptors have string representations.
    //
    // Example (CSS, taken from MDN):
    //
    //   font: 1.2rem "Fira Sans" sans-serif;
    //
    // Here we use a format similar to that of Pango.
    //
    // Pango's format:
    //
    //   font-descriptor: family-list? style-options? size? variations? features?
    //
    //   family-list: family ("," family)* ","?
    //
    //   style-options: style-option (" " style-option)*
    //
    //   style-option: style | variant | weight | stretch | gravity
    //
    //   style: "normal" | "roman" | "oblique" | "italic"
    //
    //   variants: "small-caps" | "all-small-caps" | "petite-caps" | "all-petite-caps" | "unicase" |
    //   "title-caps"
    //
    //   weight: "thin" | ...
    //
    //   stretch: "ultra-condensed" | ...
    //
    //   gravity: "not-rotated" | "south" | ...
    //
    //   variations: @axis1=value1, @axis2=value2, ...
    //
    //   features: #feature1=value1, #feature2=value2, ...
    //
    // Example:
    //
    //   Cantarell Italic Light 15 @wght=200 #tnum=1
    //
    // Note: font-stretch is renamed to font-width in CSS.
    //
    struct FontDescriptor {
        // Parse the font description.
        static FontDescriptor fromString(base::StringView str);

        // Provide default values for each selector.
        void normalize() {
            if (!size.hasValue()) {
                size = 12.0f;
            }
            // TODO: Other fields.
        }

        base::Optional<FontFamilyList> family_list;

        // Size of the font. The unit is point per em.
        base::Optional<float> size;

        base::Optional<FontVariant> variant;

        // TODO: Weight.
    };

}  // namespace spargel::text
