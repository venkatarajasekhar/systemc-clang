#include "SuspensionAutomata.h"
#include "FindArgument.h"
using namespace scpar;

SusCFG::SusCFG(CFGBlock * block):_block(block),
_parentCFGBlock(NULL),
_parentSusCFGBlock(NULL), _isWaitBlock(false), _isParentBlock(false)
{

}

SusCFG::~SusCFG()
{
}

void SusCFG::addParentBlock(CFGBlock * parentBlock)
{
  _parentCFGBlock = parentBlock;
}

void SusCFG::addParentBlock(SusCFG * parentBlock)
{
  _parentSusCFGBlock = parentBlock;
}

void SusCFG::addPredBlocks(SusCFG * block)
{
  _predBlocks.push_back(block);
}

void SusCFG::addSuccBlocks(SusCFG * block)
{
  _succBlocks.push_back(block);
}

void SusCFG::addChildBlockList(SusCFG * block)
{
  _childBlockList.push_back(block);
}

void SusCFG::addChildBlockList(vector < SusCFG * >blockList)
{
  for (unsigned int i = 0; i < blockList.size(); i++) {
    _childBlockList.push_back(blockList.at(i));
  }
}

void SusCFG::setWaitBlock()
{
  _isWaitBlock = true;
}

void SusCFG::setParentBlock()
{
  _isParentBlock = true;
}

void SusCFG::setWaitStmt(Stmt * stmt)
{
  _waitStmt = stmt;
}

vector < SusCFG * >SusCFG::getChildBlockList()
{
  return _childBlockList;
}

vector < SusCFG * >SusCFG::getPredBlocks()
{
  return _predBlocks;
}

vector < SusCFG * >SusCFG::getSuccBlocks()
{
  return _succBlocks;
}

unsigned int SusCFG::getBlockID()
{
  return _block->getBlockID();
}

bool SusCFG::isWaitBlock()
{
  return _isWaitBlock;
}

bool SusCFG::isParentBlock()
{
  return _isParentBlock;
}

unsigned int SusCFG::getParentBlockID()
{
  if (_parentCFGBlock) {
    return _parentCFGBlock->getBlockID();
  } else {
    return 0;
  }
}

CFGBlock *SusCFG::getBlock()
{
  return _block;
}

CFGBlock *SusCFG::getParentCFGBlock()
{
  return _parentCFGBlock;
}

SusCFG *SusCFG::getParentSusCFGBlock()
{
  return _parentSusCFGBlock;
}

Stmt *SusCFG::getWaitStmt()
{
  return _waitStmt;
}

////////////////////////////////////////////////////////////////////////////////////////
State::State(SusCFG * susCFG, bool isTimed, bool isDelta, bool isInitial, bool isEvent):_susCFGBlock(susCFG),
_isTimed(isTimed),
_isDelta(isDelta), _isInitial(isInitial), _isEvent(isEvent)
{

}

State::~State()
{
  delete _susCFGBlock;
}

bool State::isTimed()
{
  return _isTimed;
}

bool State::isDelta()
{
  return _isDelta;
}

bool State::isEvent()
{
  return _isEvent;
}

bool State::isInitial()
{
  return _isInitial;
}

void State::setInitial()
{
  _isInitial = true;
}

void State::setTimed()
{
  _isTimed = true;
}

void State::setEvent()
{

  _isEvent = true;
}

void State::setDelta()
{

  _isDelta = true;
}

void State::addEventName(string eventName)
{
  _eventName = eventName;
}

void State::addSimTime(float simTime)
{
  _timeInNS = simTime;
}

string State::getEventName()
{
  return _eventName;
}

float State::getSimTime()
{
  return _timeInNS;
}

SusCFG *State::returnSusCFGBlock()
{
  return _susCFGBlock;
}

////////////////////////////////////////////////////////////////////////////////////////
Transition::Transition():_initialState(NULL), _finalState(NULL)
{

}

Transition::~Transition()
{
  delete _initialState;
  delete _finalState;
}

void
 Transition::addInitialState(State * state)
{
  _initialState = state;
}

void Transition::addFinalState(State * state)
{
  _finalState = state;
}

void Transition::addCodeBlocks(SusCFG * susCFG)
{
  _codeBlockVector.push_back(susCFG);
}

void Transition::addCodeBlocks(vector < SusCFG * >susCFG)
{
  for (unsigned int i = 0; i < susCFG.size(); i++) {
    _codeBlockVector.push_back(susCFG.at(i));
  }
}

State *Transition::returnInitialState()
{
  return _initialState;
}

State *Transition::returnFinalState()
{
  return _finalState;
}

vector < SusCFG * >Transition::returnCodeBlocks()
{
  return _codeBlockVector;
}

void Transition::dump(raw_ostream & os)
{
  os << "\n ####### Transition #######";
  os << "\n Initial State : " << _initialState->
    returnSusCFGBlock()->getBlockID();
  os << "\n Final State : " << _finalState->returnSusCFGBlock()->getBlockID();
  os << "\n Transition Blocks : ";
  for (unsigned int i = 0; i < _codeBlockVector.size(); i++) {
    os << _codeBlockVector.at(i)->getBlockID() << " ";
  }
}

////////////////////////////////////////////////////////////////////////////////////////
SuspensionAutomata::SuspensionAutomata(FindWait::waitListType waitCalls, CXXMethodDecl * d, ASTContext * a, raw_ostream & os):_d(d), _a(a), _os(os),
_cfg
(NULL)
{
  _waitCalls = waitCalls;
}

SuspensionAutomata::~SuspensionAutomata()
{
}

void
 SuspensionAutomata::initialize()
{
  const CFG::BuildOptions & b = CFG::BuildOptions();

  _cfg = CFG::buildCFG(cast < Decl > (_d), _d->getBody(), _a, b);

}


bool SuspensionAutomata::isWaitCall(const CFGStmt * cs)
{
  bool f = false;

  Stmt *s = const_cast < Stmt * >(cs->getStmt());

  CXXMemberCallExpr *m = dyn_cast < CXXMemberCallExpr > (s);

  if (!m) {
    return f;
  }
  for (int i = 0; i < _waitCalls.size(); i++) {
    CallExpr *ce = _waitCalls.at(i);
    if (m == ce) {
      f = true;
    }
  }
/*
  for (EntryFunctionContainer::waitContainerListType::iterator it =
      _waitCalls.begin(), eit = _waitCalls.end(); it != eit; ++it) 
    WaitContainer *w = (*it);
    if (m == w->getASTNode()) {
      f = true;
    }
 */
  return f;
}

/*************************************************************************
Description : Locates the suspension points in the CFG and 
creates new CFG blocks that isolates the suspension statements from the code.
*************************************************************************/
void SuspensionAutomata::genSusCFG()
{

  LangOptions LO;

  LO.CPlusPlus = true;
  PrintingPolicy Policy(LO);

  vector < Optional < CFGStmt > >pre;


  vector < unsigned int >waitBlockIDVector;

  vector < const CFGBlock *>CFGBlockVector;;

  typedef map < CFGBlock *, SusCFG * >susCFGBlockMapType;
  typedef pair < CFGBlock *, SusCFG * >susCFGBlockPairType;

  susCFGBlockMapType susCFGBlockMap;

  bool isFirstCFGBlockID = false;

  for (CFG::iterator it = _cfg->end() - 1, eit = _cfg->begin(); it != eit; --it) {

    const CFGBlock *b = *it;
    SusCFG *currBlock;

    if (susCFGBlockMap.find(const_cast < CFGBlock * >(b)) ==
        susCFGBlockMap.end()) {
      currBlock = new SusCFG(const_cast < CFGBlock * >(b));
      susCFGBlockMap.insert(susCFGBlockPairType
                            (const_cast < CFGBlock * >(b), currBlock));
    } else {
      susCFGBlockMapType::iterator susCFGBlockFound =
        susCFGBlockMap.find(const_cast < CFGBlock * >(b));
      currBlock = susCFGBlockFound->second;
    }

    bool foundWait = false;

    vector < CFGBlock * >splitBlocksVector;
    _os << "==========================================================\n";
    _os << "Dump CFG Block\n";
    b->dump(_cfg, LO, false);

    unsigned int prevCFGBlockID;

    for (CFGBlock::const_iterator bit = b->begin(), bite = b->end();
         bit != bite; bit++) {
      if (Optional < CFGStmt > cs = bit->getAs < CFGStmt > ()) {
        const CFGStmt *s = (CFGStmt const *) &cs;

        if (isWaitCall(s)) {

          foundWait = true;
          CFGBlock *newSplitBlock = _cfg->createBlock();

          for (unsigned int i = 0; i < pre.size(); i++) {
            newSplitBlock->appendStmt(const_cast <
                                      Stmt *
                                      >(pre.at(i)->getStmt()),
                                      _cfg->getBumpVectorContext());
          }

          CFGBlock *waitBlock = _cfg->createBlock();

          waitBlock->appendStmt(const_cast <
                                Stmt * >(cs->getStmt()),
                                _cfg->getBumpVectorContext());
          waitBlock->setLabel(const_cast < Stmt * >(cs->getStmt()));
          splitBlocksVector.push_back(newSplitBlock);
          splitBlocksVector.push_back(waitBlock);
          pre.clear();

        } else {
          pre.push_back(cs);
        }
      }

    }
    //pre.clear();

    if (foundWait) {

      currBlock->setParentBlock();
      CFGBlock *prev = NULL;
      SusCFG *prevBlock = NULL;

      for (unsigned int i = 0; i < splitBlocksVector.size(); i++) {
        CFGBlock *current = splitBlocksVector.at(i);
        SusCFG *splitBlock =
          new SusCFG(const_cast < CFGBlock * >(splitBlocksVector.at(i)));
        currBlock->addChildBlockList(splitBlock);
        if (current->getLabel()) {
          splitBlock->setWaitBlock();
          splitBlock->setWaitStmt(current->getLabel());
        }
        if (pre.size() != 0) {
          // add trailing statements to post block wait stmt
          CFGBlock *newCFGBlock = _cfg->createBlock();
          for (unsigned int j = 0; j < pre.size(); j++) {
            newCFGBlock->appendStmt(const_cast < Stmt * >(pre.at(j)->getStmt()),
                                    _cfg->getBumpVectorContext());
          }
          //SusCFG * postSusCFGBlock = new SusCFG(newCFGBlock);
          //susCFGBlockMap.insert(susCFGBlockPairType(newCFGBlock, postSusCFGBlock));
          splitBlocksVector.push_back(newCFGBlock);
          pre.clear();
        }
        if (i == 0) {
          for (CFGBlock::const_pred_iterator pit =
               b->pred_begin(), pite = b->pred_end(); pit != pite; pit++) {
            const CFGBlock *pred = *pit;

            if (susCFGBlockMap.find
                (const_cast < CFGBlock * >(pred)) == susCFGBlockMap.end()) {
              SusCFG *tmpBlock = new SusCFG(const_cast < CFGBlock * >(pred));
              susCFGBlockMap.insert(susCFGBlockPairType
                                    (const_cast <
                                     CFGBlock * >(pred), tmpBlock));
              splitBlock->addPredBlocks(tmpBlock);
            } else {
              susCFGBlockMapType::iterator susCFGBlockFound =
                susCFGBlockMap.find(const_cast < CFGBlock * >(pred));
              splitBlock->addPredBlocks(susCFGBlockFound->second);
            }
          }

          splitBlock->addParentBlock(const_cast < CFGBlock * >(b));
          splitBlock->addParentBlock(currBlock);
          prevBlock = splitBlock;
        } else if (i == splitBlocksVector.size() - 1) {
          prev = splitBlocksVector.at(i - 1);
          if (susCFGBlockMap.find(prev) == susCFGBlockMap.end()) {
            SusCFG *tmpBlock = new SusCFG(const_cast < CFGBlock * >(prev));
            susCFGBlockMap.insert(susCFGBlockPairType
                                  (const_cast < CFGBlock * >(prev), tmpBlock));
            splitBlock->addPredBlocks(tmpBlock);
          } else {
            susCFGBlockMapType::iterator susCFGBlockFound =
              susCFGBlockMap.find(const_cast < CFGBlock * >(prev));
            splitBlock->addPredBlocks(susCFGBlockFound->second);
          }
          prevBlock->addSuccBlocks(splitBlock);

          for (CFGBlock::const_succ_iterator sit =
               b->succ_begin(), site = b->succ_end(); sit != site; sit++) {
            const CFGBlock *succ = *sit;

            if (susCFGBlockMap.find
                (const_cast < CFGBlock * >(succ)) == susCFGBlockMap.end()) {
              SusCFG *tmpBlock = new SusCFG(const_cast < CFGBlock * >(succ));
              susCFGBlockMap.insert(susCFGBlockPairType
                                    (const_cast <
                                     CFGBlock * >(succ), tmpBlock));
              splitBlock->addSuccBlocks(tmpBlock);
            } else {
              susCFGBlockMapType::iterator susCFGBlockFound =
                susCFGBlockMap.find(const_cast < CFGBlock * >(succ));
              splitBlock->addSuccBlocks(susCFGBlockFound->second);
            }
          }
        } else {
          prev = splitBlocksVector.at(i - 1);
          if (susCFGBlockMap.find
              (const_cast < CFGBlock * >(prev)) == susCFGBlockMap.end()) {
            SusCFG *tmpBlock = new SusCFG(const_cast < CFGBlock * >(prev));
            susCFGBlockMap.insert(susCFGBlockPairType
                                  (const_cast < CFGBlock * >(prev), tmpBlock));
            splitBlock->addPredBlocks(tmpBlock);
          } else {
            susCFGBlockMapType::iterator susCFGBlockFound =
              susCFGBlockMap.find(const_cast < CFGBlock * >(prev));
            splitBlock->addPredBlocks(susCFGBlockFound->second);
          }
          prevBlock->addSuccBlocks(splitBlock);
          prevBlock = splitBlock;
        }
        _susCFGVector.push_back(splitBlock);
      }
    } else {
      for (CFGBlock::const_pred_iterator pit =
           b->pred_begin(), pite = b->pred_end(); pit != pite; pit++) {
        const CFGBlock *predBlock = *pit;

        if (predBlock) {
          if (susCFGBlockMap.find
              (const_cast < CFGBlock * >(predBlock)) == susCFGBlockMap.end()) {
            SusCFG *tmpBlock = new SusCFG(const_cast < CFGBlock * >(predBlock));
            susCFGBlockMap.insert(susCFGBlockPairType
                                  (const_cast <
                                   CFGBlock * >(predBlock), tmpBlock));
            currBlock->addPredBlocks(tmpBlock);
          } else {
            susCFGBlockMapType::iterator predSusCFGBlockFound =
              susCFGBlockMap.find(const_cast < CFGBlock * >(predBlock));
            currBlock->addPredBlocks(predSusCFGBlockFound->second);
          }
        }
      }
      for (CFGBlock::const_succ_iterator sit =
           b->succ_begin(), site = b->succ_end(); sit != site; sit++) {
        const CFGBlock *succBlock = *sit;
        SusCFG *tmpBlock;

        if (succBlock) {
          if (susCFGBlockMap.find
              (const_cast < CFGBlock * >(succBlock)) == susCFGBlockMap.end()) {
            tmpBlock = new SusCFG(const_cast < CFGBlock * >(succBlock));
            susCFGBlockMap.insert(susCFGBlockPairType
                                  (const_cast <
                                   CFGBlock * >(succBlock), tmpBlock));
            currBlock->addSuccBlocks(tmpBlock);
          } else {
            susCFGBlockMapType::iterator succSusCFGBlockFound =
              susCFGBlockMap.find(const_cast < CFGBlock * >(succBlock));
            currBlock->addSuccBlocks(succSusCFGBlockFound->second);
          }
        }
      }
      _susCFGVector.push_back(currBlock);
    }
  }

}

/*
void SuspensionAutomata::genSauto()
{
  typedef map<SusCFG*, vector<SusCFG*> > stateCommonBlocksMapType;
  typedef pair<SusCFG*, vector<SusCFG*> > stateCommonBlocksPairType;

  typedef map < SusCFG *, State * > susCFGBlockStateMapType;
  typedef pair < SusCFG *, State * > susCFGBlockStatePairType;

    deque < SusCFG * >stateQueue;
    vector < SusCFG * >visitedStates;
    vector < SusCFG * >visitedWaitStates;
    deque < SusCFG * >waitStates;

    susCFGVectorType susCFGVector = _susCFGVector;

    susCFGBlockStateMapType susCFGBlockStateMap;
    stateCommonBlocksMapType stateCommonBlocksMap; 
    waitStates.push_back(susCFGVector.at(0));
    while (!waitStates.empty()) {
      stateQueue.clear();
      visitedStates.clear();
      vector < SusCFG * >tmpCodeBlocks;
      State *initial;
      State *finalState;
      visitedWaitStates.push_back(waitStates.back());
      if (waitStates.back()->getParentSusCFGBlock()) {
        visitedWaitStates.push_back(waitStates.back()->getParentSusCFGBlock());
      }
      if (susCFGBlockStateMap.find(waitStates.back()) ==
          susCFGBlockStateMap.end()) {
        initial = new State(waitStates.back(), false, false, true, false);
        susCFGBlockStateMap.insert(susCFGBlockStatePairType
                                   (waitStates.back(), initial));
      } else {
        susCFGBlockStateMapType::iterator stateFound =
          susCFGBlockStateMap.find(waitStates.back());
        initial = stateFound->second;
      }
      vector < SusCFG * >succBlocks = waitStates.back()->getSuccBlocks();
      for (unsigned int i = 0; i < succBlocks.size(); i++) {

        stateQueue.push_front(succBlocks.at(i));
        visitedStates.push_back(succBlocks.at(i));
      }

      while (!stateQueue.empty()) {
        SusCFG *currentState = stateQueue.front();

        stateQueue.pop_front();
        if (currentState->isParentBlock()) {    // has a wait statement        
          vector < SusCFG * >childListBlock = currentState->getChildBlockList();
          SusCFG *finalWaitBlock;
          State *initialTmp = initial;

          for (unsigned int i = 0; i < childListBlock.size(); i++) {
            if (childListBlock.at(i)->isWaitBlock()) {
              bool isTimed = false;
              bool isDelta = false;
              bool isEvent = false;
              bool isInitial = false;
              float timeInNs;
              string eventName;

              if (isTimedWait(childListBlock.at(i)->getWaitStmt())){
                isTimed = true;
                timeInNs = getTime(childListBlock.at(i)->getWaitStmt());
              }
              else if (isEventWait(childListBlock.at(i)->getWaitStmt())){
                isEvent = true;
                eventName = getEvent(childListBlock.at(i)->getWaitStmt());
              }
              else if (isDeltaWait(childListBlock.at(i)->getWaitStmt()))
                isDelta = true;
              Transition *t = new Transition();
              //State *finalState;

              if (susCFGBlockStateMap.find(childListBlock.at(i))
                  == susCFGBlockStateMap.end()) {
                finalState =
                  new State(childListBlock.at(i), isTimed,
                            isDelta, isInitial, isEvent);
                susCFGBlockStateMap.insert
                  (susCFGBlockStatePairType(childListBlock.at(i), finalState));
              } else {
                susCFGBlockStateMapType::iterator stateFound =
                  susCFGBlockStateMap.find(childListBlock.at(i));
                finalState = stateFound->second;
              }
              finalState->addEventName(eventName);
              finalState->addSimTime(timeInNs);
              t->addInitialState(initialTmp);
              t->addFinalState(finalState);
              t->addCodeBlocks(tmpCodeBlocks);
              _transitionVector.push_back(t);
               tmpCodeBlocks.clear();
              initialTmp = finalState;
              finalWaitBlock = childListBlock.at(i);
              //break; // this break statement will cause wait blocks in a sequence to be under detected.... 
            } else {
              tmpCodeBlocks.push_back(childListBlock.at(i));
            }
          }
          if (!isFound(visitedWaitStates, finalWaitBlock)) {
            waitStates.push_front(finalWaitBlock);
          }
        } else {
          tmpCodeBlocks.push_back(currentState);

          vector < SusCFG * >succCurrentBlocks = currentState->getSuccBlocks();
          for (unsigned int i = 0; i < succCurrentBlocks.size(); i++) {
            if (!isFound(visitedStates, succCurrentBlocks.at(i))) {

              stateQueue.push_front(succCurrentBlocks.at(i));
              visitedStates.push_back(succCurrentBlocks.at(i));
            }
          }
        }
      }
      if (tmpCodeBlocks.size() != 0) {
       if (stateCommonBlocksMap.find(finalState->returnSusCFGBlock()) != stateCommonBlocksMap.end()) { 
         stateCommonBlocksMapType::iterator stateFound = stateCommonBlocksMap.find(finalState->returnSusCFGBlock());
         vector<SusCFG*> susCFGVec = stateFound->second;
         for (int i = 0; i<tmpCodeBlocks.size(); i++) {
          if (!isFound(susCFGVec, tmpCodeBlocks.at(i))) {
           susCFGVec.push_back(tmpCodeBlocks.at(i));
          }
         }
        stateCommonBlocksMap.erase(finalState->returnSusCFGBlock());
        stateCommonBlocksMap.insert(stateCommonBlocksPairType(finalState->returnSusCFGBlock(), susCFGVec)); 
       }
       else {
        stateCommonBlocksMap.insert(stateCommonBlocksPairType(finalState->returnSusCFGBlock(), tmpCodeBlocks));
       }
      }
      waitStates.pop_back();
    }    
  
   
    
    for (int i = 0; i<_transitionVector.size(); i++) {
     
      Transition *t = _transitionVector.at(i);
      if (stateCommonBlocksMap.find(t->returnFinalState()->returnSusCFGBlock()) != stateCommonBlocksMap.end()) {
       stateCommonBlocksMapType::iterator stateFound = stateCommonBlocksMap.find(t->returnFinalState()->returnSusCFGBlock());
       vector<SusCFG*> remainingBlocks = stateFound->second;
       t->addCodeBlocks(remainingBlocks);
      }
    }    
}
*/

void SuspensionAutomata::genSauto()
{

  typedef pair < SusCFG *, State * >susCFGStatePairType;
  typedef map < SusCFG *, State * >susCFGStateMapType;

  susCFGStateMapType susCFGStateMap;

  susCFGVectorType susCFGVector = _susCFGVector;
  susCFGVectorType waitBlocks;
  for (int i = 0; i < susCFGVector.size(); i++) {
    if (susCFGVector.at(i)->isWaitBlock() || i == 0) {
      waitBlocks.push_back(susCFGVector.at(i));      
      State *state = new State(susCFGVector.at(i), false, false, false, false);
      if (i == 0) {
        state->setInitial();
      }
      susCFGStateMap.insert(susCFGStatePairType(susCFGVector.at(i), state));
    }
  }
  for (int i = 0; i < waitBlocks.size(); i++) {

    SusCFG *waitBlock = waitBlocks.at(i);    
    deque < SusCFG * >susCodeBlocks;
    vector < SusCFG * >codeBlocks;      // transtion code blocks distinct for a wait block
    vector < SusCFG * >commonBlocks;    // common code blocks for all destination wait calls
    susCFGVectorType succBlocks = waitBlock->getSuccBlocks();
    susCFGStateMapType::iterator stateFound =
      susCFGStateMap.find(waitBlocks.at(i));
    State *initialState = stateFound->second;
    for (int j = 0; j < succBlocks.size(); j++) {
      susCodeBlocks.push_back(succBlocks.at(j));      
    }
    while (susCodeBlocks.size() != 0) {
      SusCFG *currentBlock = susCodeBlocks.front();
      susCodeBlocks.pop_front();
      if (currentBlock->isWaitBlock()) {
        float timeInNs;
        string eventName;

        susCFGStateMapType::iterator stateFound =
          susCFGStateMap.find(currentBlock);
        State *finalState = stateFound->second;
        
        if (isTimedWait(currentBlock->getWaitStmt())) {
          finalState->setTimed();
          timeInNs = getTime(currentBlock->getWaitStmt());
        } else if (isEventWait(currentBlock->getWaitStmt())) {

          finalState->setEvent();
          eventName = getEvent(currentBlock->getWaitStmt());
        } else if (isDeltaWait(currentBlock->getWaitStmt())) {
          finalState->setDelta();
        } else {
          finalState->setInitial();
        }
        vector < SusCFG * >transitionBlocks;
        transitionBlocks.insert(transitionBlocks.end(), commonBlocks.begin(),
                                commonBlocks.end());
        transitionBlocks.insert(transitionBlocks.end(), codeBlocks.begin(),
                                codeBlocks.end());
        codeBlocks.clear();
        Transition *t = new Transition();
        finalState->addEventName(eventName);
        finalState->addSimTime(timeInNs);
        t->addInitialState(initialState);
        t->addFinalState(finalState);
        t->addCodeBlocks(transitionBlocks);
        _transitionVector.push_back(t);
        break;
      } else if (currentBlock->isParentBlock()) {
        susCFGVectorType childBlockList = currentBlock->getChildBlockList();
        for (int j = 0; j < childBlockList.size(); j++) {
          if (childBlockList.at(j)->isWaitBlock()) {
            float timeInNs;
            string eventName;

            susCFGStateMapType::iterator stateFound =
              susCFGStateMap.find(childBlockList.at(j));
            State *finalState = stateFound->second;
            
            if (isTimedWait(childBlockList.at(j)->getWaitStmt())) {
              finalState->setTimed();
              timeInNs = getTime(childBlockList.at(j)->getWaitStmt());
            } else if (isEventWait(childBlockList.at(j)->getWaitStmt())) {              
              finalState->setEvent();
              eventName = getEvent(childBlockList.at(j)->getWaitStmt());
            } else if (isDeltaWait(childBlockList.at(j)->getWaitStmt())) {
              finalState->setDelta();
            } else {
              finalState->setInitial();
            }
            vector < SusCFG * >transitionBlocks;
            transitionBlocks.insert(transitionBlocks.end(),
                                    commonBlocks.begin(), commonBlocks.end());
            transitionBlocks.insert(transitionBlocks.end(), codeBlocks.begin(),
                                    codeBlocks.end());
            codeBlocks.clear();
            Transition *t = new Transition();
            finalState->addEventName(eventName);
            finalState->addSimTime(timeInNs);
            t->addInitialState(initialState);
            t->addFinalState(finalState);
            t->addCodeBlocks(transitionBlocks);
            _transitionVector.push_back(t);
            break;
          } else {
            codeBlocks.push_back(childBlockList.at(j));
          }
        }
      } else {
        commonBlocks.push_back(currentBlock);
        for (int j = 0; j < currentBlock->getSuccBlocks().size(); j++) {
          susCodeBlocks.push_front(currentBlock->getSuccBlocks().at(j));
        }
      }
    }
  }
}


// need a utility class and this should be a template function
bool SuspensionAutomata::isFound(vector < SusCFG * >visitedState,
                                 SusCFG * block)
{
  bool foundBlock = false;

//  _os <<"\n Comparing : " <<block->getBlockID();
  for (unsigned int i = 0; i < visitedState.size(); i++) {
    if (visitedState.at(i) == block) {
      foundBlock = true;
      break;
    }
  }
  return foundBlock;
}

bool SuspensionAutomata::isFound(vector < Transition * >visitedState,
                                 Transition * block)
{
  bool foundBlock = false;

//  _os <<"\n Comparing : " <<block->getBlockID();
  for (unsigned int i = 0; i < visitedState.size(); i++) {
    if (visitedState.at(i) == block) {
      foundBlock = true;
      break;
    }
  }
  return foundBlock;
}


float SuspensionAutomata::getTime(Stmt * stmt)
{
  float factor;
  if (CXXMemberCallExpr * ce = dyn_cast < CXXMemberCallExpr > (stmt)) {
    if (getArgumentName(ce->getArg(1)) == "SC_FS") {
      factor = 1000000;
    } else if (getArgumentName(ce->getArg(1)) == "SC_PS") {
      factor = 1000;
    } else if (getArgumentName(ce->getArg(1)) == "SC_NS") {
      factor = 1;
    } else if (getArgumentName(ce->getArg(1)) == "SC_US") {
      factor = 0.001;
    } else if (getArgumentName(ce->getArg(1)) == "SC_MS") {
      factor = 0.000001;
    } else if (getArgumentName(ce->getArg(1)) == "SC_SEC") {
      factor = 0.0000000001;
    }
    return (atof(getArgumentName(ce->getArg(0)).c_str()) * factor);
  }
}

string SuspensionAutomata::getEvent(Stmt * stmt)
{
  if (CXXMemberCallExpr * ce = dyn_cast < CXXMemberCallExpr > (stmt)) {
    return (getArgumentName(ce->getArg(0)));
  }
}


bool SuspensionAutomata::isTimedWait(Stmt * stmt)
{

  if (CXXMemberCallExpr * ce = dyn_cast < CXXMemberCallExpr > (stmt)) {
    if (ce->getNumArgs() > 1) {
      return true;
    }
  }
  return false;
}

string SuspensionAutomata::getArgumentName(Expr * arg)
{
  if (arg == NULL)
    return string("NULL");

  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  string TypeS;

  llvm::raw_string_ostream s(TypeS);

  arg->printPretty(s, 0, Policy);
  //  _os << ", argument: " << s.str() << "\n";
  return s.str();
}

bool SuspensionAutomata::isDeltaWait(Stmt * stmt)
{

  if (CXXMemberCallExpr * ce = dyn_cast < CXXMemberCallExpr > (stmt)) {
    if (ce->getNumArgs() == 1) {
      string eventName = getArgumentName(ce->getArg(0));

      if (eventName == "SC_ZERO_TIME") {
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}

bool SuspensionAutomata::isEventWait(Stmt * stmt)
{

  if (CXXMemberCallExpr * ce = dyn_cast < CXXMemberCallExpr > (stmt)) {
    if (ce->getNumArgs() == 1) {
      return true;
    }
  }
  return false;
}

SuspensionAutomata::susCFGVectorType SuspensionAutomata::getSusCFG()
{
  return _susCFGVector;
}

SuspensionAutomata::transitionVectorType SuspensionAutomata::getSauto()
{
  return _transitionVector;
}

void SuspensionAutomata::dumpSusCFG()
{

  _os << "\n#############################################";
  susCFGVectorType susCFGVector = _susCFGVector;

  for (unsigned int i = 0; i < susCFGVector.size(); i++) {
    _os << "\n Block ID : " << susCFGVector.at(i)->getBlockID();
    _os << "\n Is Wait Block : " << susCFGVector.at(i)->isWaitBlock();
    if (susCFGVector.at(i)->getParentBlockID()) {
      _os << "\n Parent ID : " << susCFGVector.at(i)->getParentBlockID();
      SusCFG *parentBlock = susCFGVector.at(i)->getParentSusCFGBlock();

      _os << "\n Parent Block ID : " << parentBlock->getBlockID();
      _os << "\n Size of Children : " <<
        parentBlock->getChildBlockList().size();
    }
    vector < SusCFG * >predBlocks = susCFGVector.at(i)->getPredBlocks();
    vector < SusCFG * >succBlocks = susCFGVector.at(i)->getSuccBlocks();
    _os << "\n Predecessor Blocks : ";
    for (unsigned int j = 0; j < predBlocks.size(); j++) {
      if (predBlocks.at(j)) {
        _os << predBlocks.at(j)->getBlockID() << " ";
      }
    }
    _os << "\n Successor Blocks : ";
    for (unsigned int j = 0; j < succBlocks.size(); j++) {
      if (succBlocks.at(j)) {
        _os << succBlocks.at(j)->getBlockID() << " ";
      }
    }
  }

}

void SuspensionAutomata::dumpSauto()
{

  vector < Transition * >transitionVector = _transitionVector;
  _os << "\n Size of transitionVector : " << transitionVector.size();
  for (unsigned int i = 0; i < transitionVector.size(); i++) {
    Transition *t = transitionVector.at(i);

    t->dump(_os);
  }
  _os << "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";

}

//////////////////////////////////////////////////////////////////////////////
