//===-- Mico32TargetMachine.cpp - Define TargetMachine for Mico32 ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "Mico32.h"
#include "Mico32MCAsmInfo.h"
#include "Mico32TargetMachine.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegistry.h"
using namespace llvm;


#if 0
static MCAsmInfo *createMCAsmInfo(const Target &T, StringRef TT) {
  Triple TheTriple(TT);
  switch (TheTriple.getOS()) {
  default:
    return new Mico32MCAsmInfo();
  }
}


static MCStreamer *createMCStreamer(const Target &T, const std::string &TT,
                                    MCContext &Ctx, TargetAsmBackend &TAB,
                                    raw_ostream &_OS,
                                    MCCodeEmitter *_Emitter,
                                    bool RelaxAll,
                                    bool NoExecStack) {
  Triple TheTriple(TT);
  switch (TheTriple.getOS()) {
  case Triple::Darwin:
    llvm_unreachable("Mico32 does not support Darwin MACH-O format");
    return NULL;
  case Triple::MinGW32:
  case Triple::Cygwin:
  case Triple::Win32:
    llvm_unreachable("Mico32 does not support Windows COFF format");
    return NULL;
  default:
    return createELFStreamer(Ctx, TAB, _OS, _Emitter, RelaxAll,
                             NoExecStack);
  }
}
#endif

extern "C" void LLVMInitializeMico32Target() {
  // Register the target.
  RegisterTargetMachine<Mico32TargetMachine> X(TheMico32Target);

  RegisterAsmInfo<Mico32ELFMCAsmInfo> A(TheMico32Target);


#if 0
  // Register the target asm info.
  RegisterAsmInfoFn A(TheMico32Target, createMCAsmInfo);
 
  // Register the MC code emitter
  TargetRegistry::RegisterCodeEmitter(TheMico32Target,
                                      llvm::createMico32MCCodeEmitter);
#endif

#if 0
  // Register the asm backend
  TargetRegistry::RegisterAsmBackend(TheMico32Target,
                                     createMico32AsmBackend);

  // Register the object streamer
  TargetRegistry::RegisterObjectStreamer(TheMico32Target, createMCStreamer);
#endif


}

///
/// Note: DataLayout is described in:
/// http://www.llvm.org/docs/LangRef.html#datalayout
///
Mico32TargetMachine::Mico32TargetMachine(const Target &T,
                                         const std::string &TT,
                                         const std::string &FS)
  : LLVMTargetMachine(T, TT), 
    Subtarget(TT, FS),
    DataLayout("E-p:32:32:32-i8:8:32-i16:16:32"),
    InstrInfo(*this), 
    FrameLowering(Subtarget),
    TLInfo(*this), TSInfo(*this)
    //, ELFWriterInfo(*this)
{
  if (getRelocationModel() == Reloc::Default)
    setRelocationModel(Reloc::Static);

  if (getCodeModel() == CodeModel::Default)
    setCodeModel(CodeModel::Small);
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//


// Install an instruction selector pass using
// the ISelDag to generate Mico32 code.
bool Mico32TargetMachine::addInstSelector(PassManagerBase &PM,
                                          llvm::CodeGenOpt::Level OptLevel) {
  PM.add(createMico32ISelDag(*this));
  return false;
}
