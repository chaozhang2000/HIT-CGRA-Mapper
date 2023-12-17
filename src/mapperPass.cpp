#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/LoopIterator.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <set>
#include "json.h"
#include "DFG.h"
#include "generated/autoconf.h"

using namespace llvm;
using namespace std;
using json = nlohmann::json;

void addDefaultKernels(map<string, list<int>*>*);

namespace {

  struct mapperPass : public FunctionPass {

  public:
    static char ID;
    mapperPass() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addPreserved<LoopInfoWrapperPass>();
      AU.setPreservesAll();
    }

		/**
		 * Mapper enter at this function
		 */
    bool runOnFunction(Function &t_F) override {

      // Read the target function from JSON file.
			string target_function;
      ifstream i("./param.json");
      if (!i.good()) {

        cout<< "=============================================================\n";
        cout<<"\033[0;31mPlease provide a valid <param.json> in the current directory."<<endl;
        cout<<"A set of default parameters is leveraged.\033[0m"<<endl;
        cout<< "=============================================================\n";
      } else {
        json param;
        i >> param; 
				target_function = param["kernel"];
      }


      // Check existance. if the function is our target function
      if (target_function!=t_F.getName().str()) {
        cout<<"[function \'"<<t_F.getName().str()<<"\' is not our target function]\n";
        return false;
      }
      cout << "==================================\n";
      cout<<"[function \'"<<t_F.getName().str()<<"\' is our target function]\n";


      DFG* dfg = new DFG(t_F);
      // Show the count of different opcodes (IRs).
      cout << "==================================\n";
      cout << "[show opcode count]\n";
      dfg->showOpcodeDistribution();

      // Generate the DFG dot file.
      cout << "==================================\n";
      cout << "[generate dot for DFG]\n";
			bool isTrimmedDemo = true;
      dfg->generateDot(t_F, isTrimmedDemo);

      // Generate the DFG dot file.
      cout << "==================================\n";
      cout << "[generate JSON for DFG]\n";
      dfg->generateJSON();
			return true;
    }

  };
}

char mapperPass::ID = 0;
static RegisterPass<mapperPass> X("mapperPass", "DFG Pass Analyse", false, false);
