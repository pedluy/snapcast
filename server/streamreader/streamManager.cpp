/***
    This file is part of snapcast
    Copyright (C) 2014-2016  Johannes Pohl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/

#include "streamManager.h"
#include "pipeStream.h"
#include "fileStream.h"
#include "common/utils.h"
#include "common/strCompat.h"
#include "common/log.h"
#include "common/snapException.h"


using namespace std;


StreamManager::StreamManager(PcmListener* pcmListener, const std::string& defaultSampleFormat, const std::string& defaultCodec, size_t defaultReadBufferMs) : pcmListener_(pcmListener), sampleFormat_(defaultSampleFormat), codec_(defaultCodec), readBufferMs_(defaultReadBufferMs)
{
}


PcmStream* StreamManager::addStream(const std::string& uri)
{
	StreamUri streamUri(uri);

	if (streamUri.query.find("sampleformat") == streamUri.query.end())
		streamUri.query["sampleformat"] = sampleFormat_;

	if (streamUri.query.find("codec") == streamUri.query.end())
		streamUri.query["codec"] = codec_;

	if (streamUri.query.find("buffer_ms") == streamUri.query.end())
		streamUri.query["buffer_ms"] = cpt::to_string(readBufferMs_);

//	logD << "\nURI: " << streamUri.uri << "\nscheme: " << streamUri.scheme << "\nhost: "
//		<< streamUri.host << "\npath: " << streamUri.path << "\nfragment: " << streamUri.fragment << "\n";

//	for (auto kv: streamUri.query)
//		logD << "key: '" << kv.first << "' value: '" << kv.second << "'\n";

	if (streamUri.scheme == "pipe")
	{
		streams_.push_back(make_shared<PipeStream>(pcmListener_, streamUri));
		return streams_.back().get();
	}
	else if (streamUri.scheme == "file")
	{
		streams_.push_back(make_shared<FileStream>(pcmListener_, streamUri));
		return streams_.back().get();
	}
	else
	{
		throw SnapException("Unknown stream type: " + streamUri.scheme);
	}

	return NULL;
}


const std::vector<PcmStreamPtr>& StreamManager::getStreams()
{
	return streams_;
}


const PcmStreamPtr StreamManager::getDefaultStream()
{
	if (streams_.empty())
		return nullptr;

	return streams_.front();
}


const PcmStreamPtr StreamManager::getStream(const std::string& id)
{
	for (auto stream: streams_)
	{
		if (stream->getUri().id() == id)
			return stream;
	}
	return nullptr;
}


void StreamManager::start()
{
	for (auto stream: streams_)
		stream->start();
}


void StreamManager::stop()
{
	for (auto stream: streams_)
		stream->stop();
}


json StreamManager::toJson() const
{
	json result = json::array();
	for (auto stream: streams_)
		result.push_back(stream->toJson());
	return result;
}


