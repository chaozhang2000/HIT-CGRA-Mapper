/**
 * @file mapperPass.cpp
 * @author Cheng Tan and Chao Zhang
 * @brief the top file of mapper
 * @version 0.1
 */ 

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
      // Read the parameter JSON file.
      // Set the target function and loop.
      map<string, list<int>*>* functionWithLoop = new map<string, list<int>*>();//record the functionwithLoop .map<name of function,pointer of the loop_number's list>
      addDefaultKernels(functionWithLoop);//not important
      ifstream i("./param.json");
      if (!i.good()) {

        cout<< "=============================================================\n";
        cout<<"\033[0;31mPlease provide a valid <param.json> in the current directory."<<endl;
        cout<<"A set of default parameters is leveraged.\033[0m"<<endl;
        cout<< "=============================================================\n";
      } else {
        json param;
        i >> param;
 
	// Check param exist or not. the name of param bellow must be include in param.json ,this will be checked in bellow try block
	set<string> paramKeys;
	paramKeys.insert("kernel");
	//finished the param.json checking

        // Configuration for customizable CGRA.(assign date from json to variables)
				//1.assign value to functionWithLoop with kernel and targetLoopsID read from json file
        (*functionWithLoop)[param["kernel"]] = new list<int>();
        (*functionWithLoop)[param["kernel"]]->push_back(0);
				//3. assign value to execLatency , piplinedOpt,additionalFunc
      }
			//finished read data from param.json

      // Check existance. if the name of kernel read from param.json is in input .bc 
      if (functionWithLoop->find(t_F.getName().str()) == functionWithLoop->end()) {
        cout<<"[function \'"<<t_F.getName().str()<<"\' is not in our target list]\n";
        return false;
      }
      cout << "==================================\n";
      cout<<"[function \'"<<t_F.getName().str()<<"\' is one of our targets]\n";

      // TODO: will make a list of patterns/tiles to illustrate how the
      //       heterogeneity is
			map<string,int>* execLatency = new map<string,int>();
			list<string>* pipelinedOpt = new list<string>();
      DFG* dfg = new DFG(t_F,execLatency, pipelinedOpt);
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
		//end of runOnFunction

  };//end of FucntionPass class
}//end of namespace

char mapperPass::ID = 0;
static RegisterPass<mapperPass> X("mapperPass", "DFG Pass Analyse", false, false);

/**
 * Add the kernel names of some popular applications.Assume each kernel contains single loop.
 * @param t_functionWithLoop the pointer to data structure used to record the name of functions that contains loops and the labels of loops
 * 
 * the Name of kernel function need be add to functionWithLoop first.We can find kernel's name in kernel.ll,Actually we just need to add one kernel's name,but we need to test different kernels,so we add them here ahead of time. A better implementation is to pass the name of the kernel as a parameter.TODO
 */
void addDefaultKernels(map<string, list<int>*>* t_functionWithLoop) {

  (*t_functionWithLoop)["_Z12ARENA_kerneliii"] = new list<int>();
  (*t_functionWithLoop)["_Z12ARENA_kerneliii"]->push_back(0);
  (*t_functionWithLoop)["_Z4spmviiPiS_S_"] = new list<int>();
  (*t_functionWithLoop)["_Z4spmviiPiS_S_"]->push_back(0);
  (*t_functionWithLoop)["_Z4spmvPiii"] = new list<int>();
  (*t_functionWithLoop)["_Z4spmvPiii"]->push_back(0);
  (*t_functionWithLoop)["adpcm_coder"] = new list<int>();
  (*t_functionWithLoop)["adpcm_coder"]->push_back(0);
  (*t_functionWithLoop)["adpcm_decoder"] = new list<int>();
  (*t_functionWithLoop)["adpcm_decoder"]->push_back(0);
  (*t_functionWithLoop)["kernel_gemm"] = new list<int>();
  (*t_functionWithLoop)["kernel_gemm"]->push_back(0);
  (*t_functionWithLoop)["kernel"] = new list<int>();
  (*t_functionWithLoop)["kernel"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelPfS_S_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPfS_S_"]->push_back(0);
  (*t_functionWithLoop)["_Z6kerneliPPiS_S_S_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kerneliPPiS_S_S_"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelPPii"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPii"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelP7RGBType"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelP7RGBType"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelP7RGBTypePi"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelP7RGBTypePi"]->push_back(0);
  (*t_functionWithLoop)["_Z6kernelP7RGBTypeP4Vect"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelP7RGBTypeP4Vect"]->push_back(0);
  (*t_functionWithLoop)["fir"] = new list<int>();
  (*t_functionWithLoop)["fir"]->push_back(0);
  (*t_functionWithLoop)["spmv"] = new list<int>();
  (*t_functionWithLoop)["spmv"]->push_back(0);
  // (*functionWithLoop)["fir"].push_back(1);
  (*t_functionWithLoop)["latnrm"] = new list<int>();
  (*t_functionWithLoop)["latnrm"]->push_back(1);
  (*t_functionWithLoop)["fft"] = new list<int>();
  (*t_functionWithLoop)["fft"]->push_back(0);
  (*t_functionWithLoop)["BF_encrypt"] = new list<int>();
  (*t_functionWithLoop)["BF_encrypt"]->push_back(0);
  (*t_functionWithLoop)["susan_smoothing"] = new list<int>();
  (*t_functionWithLoop)["susan_smoothing"]->push_back(0);

  (*t_functionWithLoop)["_Z9LUPSolve0PPdPiS_iS_"] = new list<int>();
  (*t_functionWithLoop)["_Z9LUPSolve0PPdPiS_iS_"]->push_back(0);

  // For LU:
  // init
  (*t_functionWithLoop)["_Z6kernelPPdidPi"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdidPi"]->push_back(0);

  // solver0 & solver1
  (*t_functionWithLoop)["_Z6kernelPPdPiS_iS_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdPiS_iS_"]->push_back(0);

  // determinant
  (*t_functionWithLoop)["_Z6kernelPPdPii"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdPii"]->push_back(0);

  // invert
  (*t_functionWithLoop)["_Z6kernelPPdPiiS0_"] = new list<int>();
  (*t_functionWithLoop)["_Z6kernelPPdPiiS0_"]->push_back(0);

  // nested
  // (*t_functionWithLoop)["_Z6kernelPfS_S_"] = new list<int>();
  // (*t_functionWithLoop)["_Z6kernelPfS_S_"]->push_back(0);
}
