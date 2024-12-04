#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;
class TargetProgram;

void legalizeIRForWGSL(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink);
} // namespace Slang
