// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * macOS specific code
 */

#include "ink-osx.h"

#import <AppKit/NSWindow.h>

namespace Inkscape {
/**
 * Same as "defaults write org.inkscape.Inkscape $key $value" but only if the key is unset.
 */
static void setNSUserDefaultIfUnset(NSString *key, BOOL value)
{
    auto userdefaults = [NSUserDefaults standardUserDefaults];
    if ([userdefaults objectForKey:key] == nil) {
        [userdefaults setBool:value forKey:key];
    }
}

/**
 * Remove unwanted automatically added menu items on macOS.
 *
 * Based on https://github.com/opencor/opencor/blob/c90e38140d/src/misc/macos.mm
 */
void removeMacosSpecificMenuItems()
{
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER
    // Remove the "Show Tab Bar" menu item from the "View" menu
    [NSWindow setAllowsAutomaticWindowTabbing:NO];
#endif

    // Remove the "Enter Full Screen" menu item from the "View" menu
    setNSUserDefaultIfUnset(@"NSFullScreenMenuItemEverywhere", NO);

    // Remove the "Start Dictation..." menu item from the "Edit" menu
    setNSUserDefaultIfUnset(@"NSDisabledDictationMenuItem", YES);

    // Remove the "Emoji & Symbols" menu items from the "Edit" menu
    setNSUserDefaultIfUnset(@"NSDisabledCharacterPaletteMenuItem", YES);
}
}

// vim: filetype=objcpp:expandtab:shiftwidth=4:softtabstop=4 :
