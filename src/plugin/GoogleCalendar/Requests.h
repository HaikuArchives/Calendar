/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _REQUESTS_H
#define _REQUESTS_H

#include <HttpHeaders.h>
#include <HttpRequest.h>
#include <NetworkKit.h>
#include <UrlProtocolListener.h>
#include <UrlProtocolRoster.h>
#include <UrlRequest.h>
#include <UrlSynchronousRequest.h>
#include <HttpHeaders.h>
#include <HttpRequest.h>
#include <Json.h>


class ProtocolListener : public BUrlProtocolListener {
public:
	ProtocolListener(bool traceLogging)
		:
		fDownloadIO(NULL),
		fTraceLogging(traceLogging)
	{
	}

	virtual ~ProtocolListener()
	{
	}

	virtual	void ConnectionOpened(BUrlRequest* caller)
	{
	}

	virtual void HostnameResolved(BUrlRequest* caller, const char* ip)
	{
	}

	virtual void ResponseStarted(BUrlRequest* caller)
	{
	}

	virtual void HeadersReceived(BUrlRequest* caller, const BUrlResult& result)
	{
	}

	virtual void DataReceived(BUrlRequest* caller, const char* data,
		off_t position, ssize_t size)
	{
		if (fDownloadIO != NULL)
			fDownloadIO->Write(data, size);
	}

	virtual	void DownloadProgress(BUrlRequest* caller, ssize_t bytesReceived,
		ssize_t bytesTotal)
	{
	}

	virtual void UploadProgress(BUrlRequest* caller, ssize_t bytesSent,
		ssize_t bytesTotal)
	{
	}

	virtual void RequestCompleted(BUrlRequest* caller, bool success)
	{
	}

	virtual void DebugMessage(BUrlRequest* caller,
		BUrlProtocolDebugMessage type, const char* text)
	{
		if (fTraceLogging)
			printf("jrpc: %s\n", text);
	}

	void SetDownloadIO(BDataIO* downloadIO)
	{
		fDownloadIO = downloadIO;
	}

private:
	BDataIO*		fDownloadIO;
	bool			fTraceLogging;
};


class Requests {
	public:
		static status_t Request(BString url, const char* const method,
			BHttpHeaders* headers, BHttpForm* form, const BString* jsonString,
			BMessage& responseMessage)
		{
			ProtocolListener listener(true);
			BUrl link(url);
			BUrlRequest* request = BUrlProtocolRoster::MakeRequest( link, &listener );
			BHttpRequest* hRequest = dynamic_cast<BHttpRequest *>(request);

			hRequest->SetMethod(method);

			if (headers != NULL)
				hRequest->SetHeaders(*headers);

			if (form != NULL)
				hRequest->SetPostFields(*form);

			if (jsonString != NULL) {
				BMemoryIO* data = new BMemoryIO(
				jsonString->String(), jsonString->Length() - 1);
				hRequest->AdoptInputData(data, jsonString->Length() - 1);
			}

			BMallocIO replyData;
			listener.SetDownloadIO(&replyData);

			thread_id thread = request->Run();
			wait_for_thread(thread, NULL);

			BString responseJson;

			const BHttpResult& result = dynamic_cast<const BHttpResult&>(
				request->Result());

			int32 statusCode = result.StatusCode();
			if (statusCode != 200) {
				printf("Response code:  %d \n", statusCode);
				return B_ERROR;
			}

			responseJson.SetTo(static_cast<const char*>(replyData.Buffer()),
				replyData.BufferLength());

			if (responseJson.Length() == 0) {
				printf("No Json data found in response \n");
				return B_ERROR;
			}

			status_t status = BJson::Parse(responseJson, responseMessage);
			if (status == B_BAD_DATA) {
				printf("Parser choked on JSON:\n%s\n", responseJson.String());
				return B_ERROR;
			}

			return B_OK;
		}
};

#endif
