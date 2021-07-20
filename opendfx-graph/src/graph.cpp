// wrapper.cpp
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include "graph.hpp"
#include "accel.hpp"
#include "buffer.hpp"
#include "link.hpp"

using opendfx::Graph;

Graph::Graph(const std::string &name) : m_name(name) {
	srand(time(0));
	id = rand() % 0x10000;
	std::stringstream stream;
	stream << std::hex << id;
	strid = stream.str();
}

std::string Graph::getInfo() const {
	return "{\"name\": \"" + m_name + "_" + strid + "\"}";
}

opendfx::Accel* Graph::addAccel(const std::string &name){
	opendfx::Accel *accel = new opendfx::Accel(name);
	accels.push_back(accel);
	return accel;
}

opendfx::Buffer* Graph::addBuffer(const std::string &name){
	opendfx::Buffer *buffer= new opendfx::Buffer(name);
	buffers.push_back(buffer);
	return buffer;
}

opendfx::Link* Graph::addInputBuffer(opendfx::Accel *accel, opendfx::Buffer *buffer){
	opendfx::Link *link = new opendfx::Link(accel, buffer, opendfx::direction::toAccel);
	links.push_back(link);
	return link;
}

opendfx::Link* Graph::addOutputBuffer(opendfx::Accel *accel, opendfx::Buffer *buffer){
	opendfx::Link *link = new opendfx::Link(accel, buffer, opendfx::direction::fromAccel);
	links.push_back(link);
	return link;
}

int Graph::delAccel(opendfx::Accel *accel){
	for (std::vector<opendfx::Link *>::iterator it = links.begin() ; it != links.end(); ++it)
	{
		if ((*it)->getAccel() == accel){
			(*it)->setDeleteFlag(true);
		}
	}
	accel->setDeleteFlag(true);
	links.erase(std::remove_if(links.begin(), links.end(), Link::staticGetDeleteFlag), links.end());
	accels.erase(std::remove(accels.begin(), accels.end(), &accel), accels.end());
	accels.erase(std::remove_if(accels.begin(), accels.end(), Accel::staticGetDeleteFlag), accels.end());
	return 0;
}

int Graph::delBuffer(opendfx::Buffer *buffer){
	for (std::vector<opendfx::Link *>::iterator it = links.begin() ; it != links.end(); ++it)
	{
		if ((*it)->getBuffer() == buffer){
			(*it)->setDeleteFlag(true);
		}
	}
	buffer.setDeleteFlag(true);
	links.erase(std::remove_if(links.begin(), links.end(), Link::staticGetDeleteFlag), links.end());
	buffers.erase(std::remove_if(buffers.begin(), buffers.end(), Buffer::staticGetDeleteFlag), buffers.end());
	return 0;
}

int Graph::delLink(opendfx::Link &link){
	links.erase(std::remove(links.begin(), links.end(), &link), links.end());
	return 0;
}

int Graph::listAccels()
{
	for (std::vector<opendfx::Accel *>::iterator it = accels.begin() ; it != accels.end(); ++it)
	{
		opendfx::Accel* accel = *it;
		std::cout << ' ' << accel->info() << '\n';
	}
	std::cout << '\n';
	return 0;
}

int Graph::listBuffers()
{
	for (std::vector<opendfx::Buffer *>::iterator it = buffers.begin() ; it != buffers.end(); ++it)
	{
		opendfx::Buffer* buffer = *it;
		std::cout << ' ' << buffer->info() << '\n';
	}
	std::cout << '\n';
	return 0;
}

int Graph::listLinks()
{
	for (std::vector<opendfx::Link *>::iterator it = links.begin() ; it != links.end(); ++it)
	{
		opendfx::Link* link = *it;
		link->info();
	}
	std::cout << '\n';
	return 0;
}

std::string Graph::jsonAccels(bool withDetail)
{
	std::stringstream jsonStream;
	jsonStream << "[ ";
	bool first = true;
	for (std::vector<opendfx::Accel *>::iterator it = accels.begin() ; it != accels.end(); ++it)
	{
		if (first){
			first = false;
			jsonStream << "\n";
		}
		else{
			jsonStream << ",\n";
		}
		jsonStream << "\t" << (*it)->toJson(withDetail);
	}
	jsonStream << "]";
	return jsonStream.str();
}

std::string Graph::jsonBuffers(bool withDetail)
{
	std::stringstream jsonStream;
	jsonStream << "[ ";
	bool first = true;
	for (std::vector<opendfx::Buffer *>::iterator it = buffers.begin() ; it != buffers.end(); ++it)
	{
		if (first){
			first = false;
			jsonStream << "\n";
		}
		else{
			jsonStream << ",\n";
		}
		jsonStream << "\t" << (*it)->toJson(withDetail);
	}
	jsonStream << "]";
	return jsonStream.str();
}

std::string Graph::jsonLinks(bool withDetail)
{
	std::stringstream jsonStream;
	jsonStream << "[ ";
	bool first = true;
	for (std::vector<opendfx::Link *>::iterator it = links.begin() ; it != links.end(); ++it)
	{
		if (first){
			first = false;
			jsonStream << "\n";
		}
		else{
			jsonStream << ",\n";
		}
		jsonStream << "\t" << (*it)->toJson(withDetail);
	}
	jsonStream << "]";
	return jsonStream.str();
}

std::string Graph::toJson(bool withDetail){
	std::stringstream jsonStream;
	jsonStream << "{\n";
	jsonStream << "\"id\"\t: "      << strid << ",\n";
	jsonStream << "\"name\"\t: "      << m_name << ",\n";
	jsonStream << "\"accels\"\t: "  << jsonAccels(withDetail) << ",\n";
	jsonStream << "\"buffers\"\t: " << jsonBuffers(withDetail) << ",\n";
	jsonStream << "\"links\"\t: "   << jsonLinks(withDetail) << "\n";
	jsonStream << "}\n";
	return jsonStream.str();
}