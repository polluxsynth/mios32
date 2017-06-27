/* -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*- */
// $Id: KnobSmall.cpp 2070 2014-09-27 23:35:11Z tk $

#include "KnobSmall.h"

//==============================================================================
KnobSmall::KnobSmall(const String &name)
    : Knob(name, 48, 48, ImageCache::getFromMemory(BinaryData::knob_small_png, BinaryData::knob_small_pngSize))
{
}

KnobSmall::~KnobSmall()
{
}
