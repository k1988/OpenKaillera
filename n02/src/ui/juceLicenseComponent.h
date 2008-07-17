/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  2 Jul 2008 3:58:22 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.11

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_JUCELICENSECOMPONENT_JUCELICENSECOMPONENT_C8ED5DE9__
#define __JUCER_HEADER_JUCELICENSECOMPONENT_JUCELICENSECOMPONENT_C8ED5DE9__

//[Headers]     -- You can add your own extra header files here --
#include "juce02.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class juceLicenseComponent  : public Component,
                              public ButtonListener
{
public:
    //==============================================================================
    juceLicenseComponent ();
    ~juceLicenseComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    TextEditor* txtLicense;
    TextButton* btnAccept;
    TextButton* btnDecline;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    juceLicenseComponent (const juceLicenseComponent&);
    const juceLicenseComponent& operator= (const juceLicenseComponent&);
};


#endif   // __JUCER_HEADER_JUCELICENSECOMPONENT_JUCELICENSECOMPONENT_C8ED5DE9__
