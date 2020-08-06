/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Application/jucer_Application.h"

//==============================================================================
class UserAvatarComponent  : public Component,
                             public SettableTooltipClient,
                             public ChangeBroadcaster,
                             private LicenseController::LicenseStateListener
{
public:
    UserAvatarComponent (bool isInteractive)
        : interactive (isInteractive)
    {
        lookAndFeelChanged();
    }

    ~UserAvatarComponent() override
    {
    }

    void paint (Graphics& g) override
    {
        auto bounds = getLocalBounds();

        bounds = bounds.removeFromRight (bounds.getHeight());
        
        Path ellipse;
        ellipse.addEllipse (bounds.toFloat());
        
        g.reduceClipRegion (ellipse);

        g.drawImage (currentAvatar, bounds.toFloat(), RectanglePlacement::fillDestination);
    }

    void mouseUp (const MouseEvent&) override
    {
        if (interactive)
        {
            PopupMenu menu;
            menu.addCommandItem (ProjucerApplication::getApp().commandManager.get(), CommandIDs::loginLogout);

            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (this));
        }
    }

private:
    //==============================================================================
    Image createStandardAvatarImage()
    {
        Image image (Image::ARGB, 250, 250, true);
        Graphics g (image);

        g.setColour (findColour (defaultButtonBackgroundColourId));
        g.fillAll();

        g.setColour (findColour (defaultIconColourId));

        auto path = getIcons().user;
        g.fillPath (path, RectanglePlacement (RectanglePlacement::centred)
                            .getTransformToFit (path.getBounds(), image.getBounds().reduced (image.getHeight() / 5).toFloat()));

        return image;
    }

    //==============================================================================
    void licenseStateChanged() override
    {
        if (interactive)
        {
            setTooltip ({});
        }

        currentAvatar = standardAvatarImage;

        repaint();
        sendChangeMessage();
    }

    void lookAndFeelChanged() override
    {
        standardAvatarImage = createStandardAvatarImage();
        signedOutAvatarImage = createStandardAvatarImage();

        if (interactive)
            signedOutAvatarImage.multiplyAllAlphas (0.4f);

        licenseStateChanged();
        repaint();
    }

    //==============================================================================
    Image standardAvatarImage, signedOutAvatarImage, currentAvatar;
    bool interactive = false;
};
