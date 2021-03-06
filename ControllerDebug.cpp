/*
 * FrameworkDebug.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: sam
 */

/**
 * This file contains methods from Framework.h that is used in debug
 */
#include "Controller.h"
//
#if !defined(EMBEDDED_MODE)
#include <mutex>
#include "Config.h"
#endif
//
#if !defined(EMBEDDED_MODE)
#include <fstream>
#include <map>
#endif

void Controller::errorHandler()
{
#if defined(EMBEDDED_MODE)
  if (errorState)
  {
    uint8_t ledErrorState = HIGH;
    unsigned long prevTime1 = millis();
    unsigned long prevTime2 = prevTime1;
    for (;;)
    {
      if (millis() - prevTime1 > 3000)
      {
        prevTime1 = millis();
        Serial.println(errorMsg);
      }
      uint8_t ledErrorState = HIGH;
      if (millis() - prevTime2 > 250)
      {
        prevTime2 = millis();
        ledErrorState ^= HIGH;
      }
      digitalWrite(RED_LED, ledErrorState);
    }
  }
#endif
}

void Controller::forcedExit(const MyString& errorMsg)
{
#if !defined(EMBEDDED_MODE)
  std::cout << "ERROR!" << errorMsg << std::endl;
  exit(0); // exit from the framework
#endif
  errorState = true;
}

void Controller::verbose()
{
#if !defined(EMBEDDED_MODE)
  std::cout << std::endl << std::endl;
// This shows the raw graph
  std::cout << "moduleVector.size()=" << moduleVector.size() << std::endl;
  for (ModuleVector::const_iterator iter = moduleVector.begin(); iter != moduleVector.end(); iter++)
  {
    const Controller::ModuleEntry* moduleEntry = *iter;
    std::cout << moduleEntry->moduleNode->getName() << std::endl;
  }

  std::cout << std::endl;
  std::cout << "representationVector.size()=" << representationVector.size() << std::endl;
  for (RepresentationVector::const_iterator iter = representationVector.begin();
      iter != representationVector.end(); iter++)
  {
    const Controller::RepresentationEntry* representationEntry = *iter;
    std::cout << representationEntry->representationNode->getName() << " "
        << representationEntry->providedModuleName << std::endl;
  }

  std::cout << std::endl;

  // One graph for all the threads
  std::cout << std::endl << std::endl;
  std::cout.flush();
  std::ofstream graph("graph.dot");

  graph << "digraph G {\n";
  int graphCounter = 0;

  for (ThreadVector::iterator iter = threadVector.begin(); iter != threadVector.end(); iter++)
  {
    Thread* thread = *iter;
    std::cout << thread->threadName << std::endl;
    std::cout << "graphStructureVector.size()=" << thread->graphStructureVector.size() << std::endl;
    std::cout << "thread->transferredVector.size()=" << thread->transferredVector.size()
        << std::endl;
    for (Thread::NodeVector::iterator iter2 = thread->graphStructureVector.begin();
        iter2 != thread->graphStructureVector.end(); iter2++)
    {
      Node* curr = *iter2;
      std::cout << "[" << curr->getName() << "] ";
      for (Node::NodeVector::iterator iter2 = curr->getNextNodes().begin();
          iter2 != curr->getNextNodes().end(); iter2++)
      {
        Node* next = *iter2;
        std::cout << "[" << next->getName() << "] ";
      }
      std::cout << std::endl;
    }

    for (Thread::NodeVector::iterator iter2 = thread->transferredVector.begin();
        iter2 != thread->transferredVector.end(); iter2++)
    {
      Node* curr = *iter2;
      std::cout << "[" << curr->getName() << "] ";
      for (Node::NodeVector::iterator iter2 = curr->getNextNodes().begin();
          iter2 != curr->getNextNodes().end(); iter2++)
      {
        Node* next = *iter2;
        std::cout << "[" << next->getName() << "] ";
      }
      std::cout << std::endl;
    }

    std::cout << "operationVector.size()=" << thread->operationVector.size() << std::endl;
    for (Thread::NodeVector::iterator iter2 = thread->operationVector.begin();
        iter2 != thread->operationVector.end(); iter2++)
    {
      const Node* x = *iter2;
      std::cout << x->getName() << std::endl;
    }

    // Graphviz output
    typedef std::map<const char*, int> LabelToIdMap;
    LabelToIdMap labelToIdMap;

    graph << "\n subgraph cluster_" << (++graphCounter) << " {\n";
    graph << "\t label=" << "\"" << thread->threadName << "\"; \n";


    for (Thread::NodeVector::const_iterator iter2 = thread->operationVector.begin();
        iter2 != thread->operationVector.end(); iter2++)
    {
      const Node* x = *iter2;
      if (x->isComputationNode())
        graph << "\t node [shape=box, fillcolor=\"green:yellow\","
            << " style=filled, penwidth=2, gradientangle=270]; \n";
      else
        graph << "\t node [shape=ellipse, fillcolor=\"gold:yellow\","
            << " style=filled, penwidth=2, gradientangle=270]; \n";

      LabelToIdMap::iterator labelToIdMapIter = labelToIdMap.find(x->getName());
      if (labelToIdMapIter == labelToIdMap.end())
        labelToIdMap.insert(std::make_pair(x->getName(), (++graphCounter)));

      graph << "\t" << labelToIdMap[x->getName()] << "[label = \"" << x->getName() << "\"]"
          << "; \n";

      //graph << " " << x->getName() << "; \n";
    }
    graph << "\n";

    for (Thread::NodeVector::iterator iter2 = thread->transferredVector.begin();
        iter2 != thread->transferredVector.end(); iter2++)
    {
      Node* x = *iter2;
      graph << "\t node [shape=ellipse, fillcolor=\"orange:yellow\","
          << " style=\"filled, dashed\" penwidth=2, gradientangle=270]; \n";

      LabelToIdMap::iterator labelToIdMapIter = labelToIdMap.find(x->getName());
      if (labelToIdMapIter == labelToIdMap.end())
        labelToIdMap.insert(std::make_pair(x->getName(), (++graphCounter)));

      graph << "\t" << labelToIdMap[x->getName()] << "[label = \"" << x->getName() << "\"]"
          << "; \n";

      //graph << " " << x->getName() << "; \n";
    }
    graph << "\n";

    // transferred Vector
    for (Thread::NodeVector::iterator iter2 = thread->transferredVector.begin();
        iter2 != thread->transferredVector.end(); iter2++)
    {
      Node* x = *iter2;
      for (Node::NodeVector::iterator j = x->getNextNodes().begin(); j != x->getNextNodes().end();
          j++)
      {
        Node* y = *j;
        if (y->isComputationNode())
        {
          graph << "edge [color=black]; \n";

          graph << "\t" << labelToIdMap[x->getName()] << " -> " << labelToIdMap[y->getName()]
              << "; \n";

          //graph << "\t" << x->getName() << " -> " << y->getName() << "; \n";
        }
      }

      if (x->getNextNodes().empty())
        graph << "\t" << labelToIdMap[x->getName()] << "[label = \"" << x->getName() << "\"]"
            << "; \n";

      //graph << "\t" << x->getName() << "; \n";
    }

    for (Thread::NodeVector::iterator iter2 = thread->operationVector.begin();
        iter2 != thread->operationVector.end(); iter2++)
    {
      Node* x = *iter2;
      for (Node::NodeVector::iterator j = x->getNextNodes().begin(); j != x->getNextNodes().end();
          j++)
      {
        Node* y = *j;
        if (x->getThreadIndex() == y->getThreadIndex())
        {
          if (y->isComputationNode())
            graph << "edge [color=black]; \n";
          else
            graph << "edge [color=blue]; \n";

          graph << "\t" << labelToIdMap[x->getName()] << " -> " << labelToIdMap[y->getName()]
              << "; \n";

          //graph << "\t" << x->getName() << " -> " << y->getName() << "; \n";
        }
      }
      if (x->getNextNodes().empty())
        graph << "\t" << labelToIdMap[x->getName()] << "[label = \"" << x->getName() << "\"]"
            << "; \n";

      //graph << "\t" << x->getName() << "; \n";

    }
    graph << "edge [color=red]; \n";
    for (Thread::NodeVector::iterator iter2 = thread->operationVector.begin();
        iter2 != thread->operationVector.end(); iter2++)
    {
      Node* x = *iter2;
      if (!x->auxiliaryNodesEmpty())
      {
        for (Node::NodeVector::iterator j = x->getAuxiliaryNodes().begin();
            j != x->getAuxiliaryNodes().end(); j++)
        {
          Node* y = *j;

          graph << "\t" << labelToIdMap[x->getName()] << " -> " << labelToIdMap[y->getName()]
              << "; \n";

          //graph << "\t" << x->getName() << " -> " << y->getName() << "; \n";
        }
      }
    }

    graph << "} \n";

  }

  graph << "\n } \n";
  graph.close();

#endif
}
