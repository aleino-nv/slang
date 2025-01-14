#include "slang-ir-legalize-atomic-operations.h"

#include "slang-ir-insts.h"

namespace Slang
{

static bool isFromDeviceBuffer(IRInst* dst)
{
    if (as<IRRWStructuredBufferGetElementPtr>(dst))
        return true;
    if (as<IRGlobalParam>(dst))
    {
        switch (dst->getDataType()->getOp())
        {
        case kIROp_GLSLShaderStorageBufferType:
            return true;
        default:
            return false;
        }
    }

    if (auto getElementPtr = as<IRGetElementPtr>(dst))
        return isFromDeviceBuffer(getElementPtr->getBase());
    if (auto fieldAddress = as<IRFieldAddress>(dst))
        return isFromDeviceBuffer(fieldAddress->getBase());

    return false;
}

static void validateAtomicOperations(DiagnosticSink* sink, IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_AtomicLoad:
    case kIROp_AtomicStore:
    case kIROp_AtomicExchange:
    case kIROp_AtomicCompareExchange:
    case kIROp_AtomicAdd:
    case kIROp_AtomicSub:
    case kIROp_AtomicAnd:
    case kIROp_AtomicOr:
    case kIROp_AtomicXor:
    case kIROp_AtomicMin:
    case kIROp_AtomicMax:
    case kIROp_AtomicInc:
    case kIROp_AtomicDec:
        {
            IRInst* destinationPtr = inst->getOperand(0);
            IRRate* destinationPtrRate = destinationPtr->getRate();
            bool isGroupShared = as<IRGroupSharedRate>(destinationPtrRate);
            bool desinationPtrValid = isGroupShared || isFromDeviceBuffer(destinationPtr);
            if (!desinationPtrValid)
            {
                sink->diagnose(inst->sourceLoc, Diagnostics::invalidAtomicDestinationPointer);
            }
        }
        break;

    default:
        break;
    }

    for (auto child : inst->getModifiableChildren())
    {
        validateAtomicOperations(sink, child);
    }
}

void legalizeAtomicOperations(DiagnosticSink * sink, IRModule* module)
{
	validateAtomicOperations(sink, module->getModuleInst());
}
} // namespace Slang
