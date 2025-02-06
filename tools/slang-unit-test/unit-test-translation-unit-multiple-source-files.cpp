#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// This creates a translation unit consisting of multiple source files.
// Make sure that imports in one of the source files apply to the entire translation unit.
SLANG_UNIT_TEST(translationUnitWithMultipleSourceFiles)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    slang::SessionDesc sessionDesc = {};
	constexpr int argc = 2;
    const char* const argv[argc] = {
        "tools/slang-unit-test/source1.slang",
        "tools/slang-unit-test/source2.slang"};
    ComPtr<slang::IBlob> sessionDescDataBlob;	
	SLANG_CHECK_ABORT(
		globalSession->parseCommandLineArguments(argc, argv, &sessionDesc, (ISlangUnknown**)sessionDescDataBlob.writeRef())
		== 
        SLANG_OK);
    ComPtr<slang::ISession> session;
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
	
    ComPtr<slang::IBlob> diagnosticBlob;
    auto module =
        session->loadModule("tools/slang-unit-test/source1.slang", diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain1", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(code != nullptr);
    SLANG_CHECK(code->getBufferSize() != 0);
}
