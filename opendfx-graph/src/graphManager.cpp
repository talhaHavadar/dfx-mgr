/*
 * Copyright (c) 2021, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
// graphManager.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <thread>
#include <atomic>
#include "utils.hpp"
#include "graph.hpp"
#include "graphManager.hpp"
#include <signal.h>
#include <dfx-mgr/daemon_helper.h>

using opendfx::GraphManager;

GraphManager::GraphManager(int slots) : slots(slots) {
	int i;
	//srand(time(0)); // Seed initialisation based on time
	//char * block;
	//short size = 1;
	std::ifstream urandom("/dev/urandom", std::ios::in|std::ios::binary); // Seed initialisation based on /dev/urandom
	urandom.read((char*)&i, sizeof(i));
	srand(i ^ time(0));
	urandom.close();
	//std::cout << i << " : " << time(0) << std::endl;
	dfx_init();
	id = rand() % 0x100000000;
}

int GraphManager::addGraph(opendfx::Graph *graph){
	int priority = graph->getPriority() % 3;
	graphQueue_mutex.lock();
	//std::cout << "addGraph " << priority << "\n";
	graphsQueue[priority].push_back(graph);
	//std::cout << "addGraph done ..\n";
	graphQueue_mutex.unlock();
	return 0;
}

int GraphManager::delGraph(opendfx::Graph *graph){
	int priority = graph->getPriority() % 3;
	graphQueue_mutex.lock();
	//std::cout << "delGraph " << priority << "\n";
	graphsQueue[priority].erase(std::remove(graphsQueue[priority].begin(), graphsQueue[priority].end(), graph), graphsQueue[priority].end());
	//std::cout << "delGraph done ..\n";
	graphQueue_mutex.unlock();
	return 0;
}

opendfx::Graph* GraphManager::getStagedGraphByID(int id)
{
	opendfx::Graph* rgraph = NULL;
	graphQueue_mutex.lock();
	for (std::vector<opendfx::Graph *>::iterator it = stagedGraphs.begin() ; it != stagedGraphs.end(); ++it)
	{
	opendfx::Graph* graph = *it;
		if (graph->getId() == id){
			rgraph = *it;
			break;
		}
	}
	graphQueue_mutex.unlock();
	return rgraph;
}

bool GraphManager::ifGraphStaged(int id){
	opendfx::Graph* graph = getStagedGraphByID(id);
	if (graph == NULL){
		return false;
	}
	else{
		return true;
	}
}

int GraphManager::listGraphs()
{
	std::vector<opendfx::Graph *>*graphs;
	for (int i = 0; i < 3; i++){
		graphQueue_mutex.lock();
		graphs = &graphsQueue[3-i-1];
		for (std::vector<opendfx::Graph *>::iterator it = graphs->begin() ; it != graphs->end(); ++it)
		{
			opendfx::Graph*graph = *it;
			std::cout << "# " << graph->getInfo() << "\n";
		}
		graphQueue_mutex.unlock();
	}
	return 0;
}

int GraphManager::mergeGraphs(){
	mergedGraph.lockAccess();
	for (std::vector<opendfx::Graph *>::iterator it = stagedGraphs.begin() ; it != stagedGraphs.end(); ++it)
	{
		opendfx::Graph* graph = *it;
		if (!graph->getStaged()){
			mergedGraph.copyGraph(graph);
			graph->allocateIOBuffers();
			graph->allocateBuffers();
			graph->allocateAccelResources();
			graph->createExecutionDependencyList();
			graph->getExecutionDependencyList();
			graph->createScheduleList();
			//graph->getScheduleListInfo();
			graph->execute();
			
			graph->setStaged(true);
			std::cout << "scheduled ..." << std::endl;
			std::cout << "No of accels  = " << mergedGraph.countAccel() << std::endl;
			std::cout << "No of buffers = " << mergedGraph.countBuffer() << std::endl; 
			std::cout << "No of links   = " << mergedGraph.countLink() << std::endl; 
			//std::cout << mergedGraph.toJson(true) << std::endl;
		}
	}
	mergedGraph.unlockAccess();
	return 0;
}

int GraphManager::stageGraphs(){
	int remainingSlots = slots;
	int accelCounts = 0;
	std::vector<opendfx::Graph *>*graphs;
	std::cout << "service started ..." << std::endl;
	while(1){
		for (int i = 0; i < 3; i++){
			graphQueue_mutex.lock();
			graphs = &graphsQueue[3-i-1];
			if (remainingSlots > 0){
				for (std::vector<opendfx::Graph *>::iterator it = graphs->begin() ; it != graphs->end(); ++it)
				{
					opendfx::Graph* graph = *it;
					accelCounts = graph->countAccel();
					if (remainingSlots > 0 && remainingSlots >= accelCounts && accelCounts > 0){
						std::cout << "##" <<  std::endl;
						remainingSlots = remainingSlots - accelCounts;
						std::cout << "staged : " << utils::int2str(graph->getId()) << std::endl;
						stagedGraphs.push_back(graph);
						std::cout << "###" << std::endl;
						graphs->erase(std::remove(graphs->begin(), graphs->end(), graph), graphs->end());
						if(graphs->size() == 0){
							break;
						}
					}
					else{
						break;
					}
				}
			}
			graphQueue_mutex.unlock();
		}
		mergeGraphs();
	}
	std::cout << "service done" << std::endl;
	return 0;
}

int GraphManager::scheduleGraph(){
	while(1){
	}
	return 0;
}

/*void GraphManager::sigint_handler (int) {
  if (pthread_self() == main_thread) {
  std::cout << "\rQuitting.\n";
  quit = true;
  for (auto &t : all) pthread_kill(t.native_handle(), SIGINT);
  } else if (!quit) pthread_kill(main_thread, SIGINT);
  }

  template <decltype(signal)>
  void GraphManager::sighandler(int sig, sighandler_t handler) {
  signal(sig, handler);
  }

  template <decltype(sigaction)>
  void GraphManager::sighandler(int sig, sighandler_t handler) {
  struct sigaction sa = {};
  sa.sa_handler = handler;
  sa.sa_flags = SA_RESTART;
  sigaction(sig, &sa, NULL);
  }
  */

int GraphManager::startServices(){
	//main_thread = pthread_self();
	stageGraphThread = new std::thread(&GraphManager::stageGraphs, this);
	// sighandler<sigaction>(SIGINT, sigint_handler);
	return 0;
}

int GraphManager::stopServices(){
	stageGraphThread->join();
	return 0;
}
