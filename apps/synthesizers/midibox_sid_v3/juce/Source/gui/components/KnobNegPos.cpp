/* -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*- */
// $Id: KnobNegPos.cpp 2070 2014-09-27 23:35:11Z tk $

#include "KnobNegPos.h"

//==============================================================================
KnobNegPos::KnobNegPos(const String &name)
    : Knob(name, 48, 48, ImageCache::getFromMemory(BinaryData::knob_negpos_png, BinaryData::knob_negpos_pngSize))
{
}

KnobNegPos::~KnobNegPos()
{
}
