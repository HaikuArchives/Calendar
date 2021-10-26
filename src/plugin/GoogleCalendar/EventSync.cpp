/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#include <sstream>
#include <time.h>

#include <Button.h>
#include <DateFormat.h>
#include <LayoutBuilder.h>
#include <List.h>
#include <Key.h>
#include <KeyStore.h>
#include <Roster.h>
#include <StringList.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>

#include "App.h"
#include "Category.h"
#include "Event.h"
#include "EventSync.h"
#include "Requests.h"
#include "QueryDBManager.h"


// Don't update status property to Google Calendar for active events(status=true)
// as we are neither allowing changing of event status, nor we are storing it. So
// we don't know whether status is "confirmed" or "tentative". Events having status
// as "cancelled" are simply deleted.
static const char* kEventStatusToGCalStatus[] = {
	"cancelled",
	"confirmed",
	"tentative",
};

class LoginDialog : public BWindow {
	public:
		LoginDialog(EventSync* sync, BString* authString)
			:
			BWindow(BRect(),"Authorization", B_TITLED_WINDOW,
				B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
		{
			BView* fMainView = new BView("MainView", B_WILL_DRAW);
			fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

			fLabel = new BStringView("AuthCodeLabel",
				"Google Calendar API Authorization");

			fAuthString = authString;

			BFont font;
			fLabel->GetFont(&font);
			font.SetSize(font.Size() * 1.4);
			fLabel->SetFont(&font, B_FONT_ALL);

			fAuthCodeText = new BTextControl("AuthCodeText", NULL,
				"Enter the Authorization Code obtained here.", NULL);

			fLogin = new BButton(NULL, "OK", new BMessage(kAuthCode));

			BLayoutBuilder::Group<>(fMainView, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(fLabel)
				.AddGlue()
			.End()
			.AddGroup(B_HORIZONTAL)
				.Add(fAuthCodeText)
			.End()
			.AddGroup(B_HORIZONTAL)
				.Add(fLogin)
			.End()
			.End();

			BLayoutBuilder::Group<>(this, B_VERTICAL)
			.SetInsets(B_USE_WINDOW_SPACING)
				.Add(fMainView)
			.End();

			ResizeTo(300, 300);
			CenterOnScreen();
		}
		~LoginDialog()
		{
		}

	private:
		void
		MessageReceived(BMessage* message)
		{
			switch(message->what)
			{
				case kAuthCode:
				{
					fAuthString->SetTo(fAuthCodeText->Text());
					Quit();
					break;
				}
				default:
					BWindow::MessageReceived(message);
			}
		}

		bool
		QuitRequested()
		{
			fAuthString->SetTo("NOT_FOUND");
			return true;
		}

		static const int kAuthCode = 1000;

		BStringView*		fLabel;
		BTextControl*		fAuthCodeText;
		BButton*		fLogin;
		BString*		fAuthString;
};


EventSync::EventSync()
	:
	fAuthCode()
{
	fDBManager = new QueryDBManager();
	fEvents = new BList();
	fCancelledEvents = new BStringList();
}


EventSync::~EventSync()
{
	delete fDBManager;
	delete fEvents;
	delete fCancelledEvents;
}


status_t
EventSync::Sync()
{
	if (LoadToken() == B_OK ) {
		if (RequestToken() != B_OK)
			RequestAuthorizationCode();
	}
	else
		RequestAuthorizationCode();

	if (GetEvents() != B_OK)
		return B_ERROR;

	if (SyncWithDatabase() !=  B_OK)
		return B_ERROR;

	return B_OK;
}


status_t
EventSync::LoadToken()
{
	BPasswordKey key;
	BKeyStore keyStore;
	if(keyStore.GetKey(kAppName, B_KEY_TYPE_PASSWORD, "refresh_token", key) == B_OK) {
		fRefreshToken = key.Password();
		return B_OK;
	}
	return B_ERROR;
}


status_t
EventSync::RequestToken()
{
	BHttpForm* form = new BHttpForm();
	form->AddString("refresh_token", fRefreshToken);
	form->AddString("client_id", CLIENT_ID);
	form->AddString("client_secret", CLIENT_SECRET);
	form->AddString("grant_type", "refresh_token");
	form->SetFormType(B_HTTP_FORM_URL_ENCODED);
	BString url("https://www.googleapis.com/oauth2/v4/token");

	BMessage refreshJson;
	Requests::Request(url, B_HTTP_GET, NULL, form, NULL, refreshJson);

	fToken = BString(refreshJson.GetString("access_token", "NOT_FOUND"));
	if (fToken.Compare("NOT_FOUND") == 0)
	{
		BPasswordKey key;
		BKeyStore keyStore;
		keyStore.GetKey(kAppName, B_KEY_TYPE_PASSWORD, "refresh_token", key);
		keyStore.RemoveKey(kAppName, key);
		return B_ERROR;
	}

	return B_OK;
}


void
EventSync::RequestAuthorizationCode()
{
	BString endpoint("https://accounts.google.com/o/oauth2/auth");
	endpoint.Append("?response_type=code");
	endpoint.Append("&client_id=");
	endpoint.Append(CLIENT_ID);
	endpoint.Append("&redirect_uri=");
	endpoint.Append(REDIRECT_URI);
	endpoint.Append("&scope=https://www.googleapis.com/auth/calendar");
	endpoint.Append("&access_type=offline");
	const char* args[] = { endpoint.String(), 0 };
	be_roster->Launch("application/x-vnd.Be.URL.http", 1, const_cast<char **>(args));
	LoginDialog* loginWindow = new LoginDialog(this, &fAuthCode);
	loginWindow->Show();

	while (fAuthCode.IsEmpty())
		snooze(1000);

	NextStep();
}


void
EventSync::NextStep()
{
	BHttpForm* form = new BHttpForm();
	form->AddString("code",fAuthCode);
	form->AddString("client_id",CLIENT_ID);
	form->AddString("client_secret",CLIENT_SECRET);
	form->AddString("grant_type","authorization_code");
	form->AddString("redirect_uri",REDIRECT_URI);
	form->SetFormType(B_HTTP_FORM_URL_ENCODED);

	BString oauth2("https://www.googleapis.com/oauth2/v3/token");
	BMessage tokenJson;

	Requests::Request(oauth2, B_HTTP_GET, NULL, form, NULL, tokenJson);

	fToken = BString(tokenJson.GetString("access_token","NOT_FOUND"));
	fRefreshToken = BString(tokenJson.GetString("refresh_token","NOT_FOUND"));

	BPasswordKey key(fRefreshToken, B_KEY_PURPOSE_WEB, "refresh_token");
	BKeyStore keyStore;
	keyStore.AddKeyring(kAppName);
	keyStore.AddKey(kAppName, key);
}


status_t
EventSync::LoadSyncToken()
{
	BPasswordKey key;
	BKeyStore keyStore;
	if(keyStore.GetKey(kAppName, B_KEY_TYPE_PASSWORD, "nextSyncToken", key) == B_OK) {
		fLastSyncToken = key.Password();
		if (fLastSyncToken.Compare("NOT_FOUND") == 0)
			return B_ERROR;
		return B_OK;
	}
	return B_ERROR;
}

status_t
EventSync::GetEvents()
{
	BString url("https://www.googleapis.com/calendar/v3/calendars/primary/events");

	if (LoadSyncToken() == B_OK) {
		url.Append("?syncToken=");
		url.Append(fLastSyncToken);
	}
	else
		url.Append("?showDeleted=true");

	BString  auth;
	auth.SetToFormat("OAuth %s", fToken.String());
	BHttpHeaders* headers = new BHttpHeaders();
	headers->AddHeader("Authorization", auth.String());

	BMessage eventJson;
	BString nextPageToken;
	BString nextSyncToken;

	do {
		if (!eventJson.IsEmpty())
			eventJson.MakeEmpty();

		if (Requests::Request(url, B_HTTP_GET, headers, NULL, NULL, eventJson)
				== B_ERROR)
			return B_ERROR;

		if (ParseEvent(&eventJson) == B_ERROR)
			return B_ERROR;

		nextPageToken = BString(eventJson.GetString("nextPageToken", "NOT_FOUND"));
		url.Append("?pageToken=");
		url.Append(nextPageToken);

	} while (nextPageToken.Compare("NOT_FOUND") != 0);

	nextSyncToken  = BString(eventJson.GetString("nextSyncToken", "NOT_FOUND"));

	BPasswordKey key(nextSyncToken, B_KEY_PURPOSE_WEB, "nextSyncToken");
	BKeyStore keyStore;
	keyStore.AddKey(kAppName, key);

	if (nextSyncToken.Compare("NOT_FOUND") == 0) {
		fprintf(stderr, "Error: nextSyncToken not found in API response.\n");
		return B_ERROR;
	}

	return B_OK;
}


status_t
EventSync::ParseEvent(BMessage* eventJson)
{
	BMessage items;
	eventJson->FindMessage("items", &items);
	int32 eventsCount = items.CountNames(B_ANY_TYPE);
	if (eventsCount == 0) {
		fprintf(stderr, "0 items found in API response.\n");
		return B_OK;
	}

	for (int32 currentEvent = 0; currentEvent < eventsCount; currentEvent++) {
		std::ostringstream ss;
		ss << currentEvent;
		BMessage event;
		if (items.FindMessage(ss.str().c_str(), &event) != B_OK) {
			fprintf(stderr, "Error: Item %d notfound in API response.\n",
				currentEvent);
			return B_ERROR;
		}

		const char* name;
		const char* place;
		const char* location;
		const char* description;
		const char* id;

		BString eventStatus;
		BString updatedString;
		BString startString;
		BString endString;

		time_t startDateTime;
		time_t endDateTime;
		time_t updated;

		uint16 status = 0;
		bool notified;

		event.FindString("id", &id);
		event.FindString("status", &eventStatus);

		if (eventStatus == BString(kEventStatusToGCalStatus[kConfirmedEvent])
			|| eventStatus == BString(kEventStatusToGCalStatus[kTentativeEvent])) {
			status = 0;
		}
		else if (eventStatus == BString(kEventStatusToGCalStatus[kCancelledEvent]))
			status |= EVENT_DELETED;

		if (status & EVENT_DELETED) {
			fCancelledEvents->Add(BString(id));
			continue;
		}

		if (event.FindString("summary", &name) != B_OK)
			name = "Untitled Event";

		if (event.FindString("location", &place) != B_OK)
			place = "";

		if (event.FindString("description", &description) != B_OK)
			description = "";

		event.FindString("updated", &updatedString);
		updated = RFC3339ToTime(updatedString, kEventUpdateDate);

		// TODO:
		// [X] Check for events having only start date (all day event).
		// All day have only the date without the time
		// Check whether reminder option is set on or off in GCal event.
		// Incorporate color IDs of GCal events into Calendar.
		// Add support for multiple calendar, treat them as a category maybe?
		bool allDay = false;
		BMessage start;
		if (event.FindMessage("start", &start) != B_OK) {
			fprintf(stderr, "Error: StartTime not found in API response.\n");
			return B_ERROR;
		}
		if (start.FindString("dateTime", &startString) != B_OK)
		if (start.FindString("date", &startString) == B_OK) {
			allDay = true;
		}
		else {
			fprintf(stderr, "Error: StartTime not found in API response.\n");
			return B_ERROR;
		}
		startDateTime = RFC3339ToTime(startString, kEventStartEndDate);


		BMessage end;
		if (event.FindMessage("end", &end) != B_OK) {
			fprintf(stderr, "Error: EndTime not found in API response.\n");
			return B_ERROR;
		}
		if (end.FindString("dateTime", &endString) != B_OK
			&& end.FindString("date", &endString) != B_OK) {
			fprintf(stderr, "Error: EndTime not found in API response.\n");
			return B_ERROR;
		}
		endDateTime = RFC3339ToTime(endString, kEventStartEndDate);
		if (allDay) endDateTime -= 86400; // if allDay remove one day from the endate

		notified = (difftime(startDateTime, BDateTime::CurrentDateTime(B_LOCAL_TIME).Time_t()) < 0) ? true : false;

		BList* categoryList = fDBManager->GetAllCategories();
		Category* category;
		for (int32 i = 0; i < categoryList->CountItems(); i++) {
			category = ((Category*)categoryList->ItemAt(i));
			if (BString(category->GetName()) == BString("Default"))
				break;
		}

		Category* newCategory = new Category(*category);

		Event* newEvent = new Event(name, place, description, allDay,
			startDateTime, endDateTime, newCategory, updated, status, id);

		fEvents->AddItem(newEvent);
	}

	return B_OK;
}


status_t
EventSync::AddEvent(Event* event)
{
	BString url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
	BString  auth;
	auth.SetToFormat("OAuth %s", fToken.String());
	BHttpHeaders* headers = new BHttpHeaders();
	headers->AddHeader("Authorization", auth.String());
	headers->AddHeader("Content-Type", "application/json");
	BString jsonString;
	jsonString.Append("{");
	jsonString += BString().SetToFormat("\"id\":\"%s\",", event->GetId());
	jsonString += BString().SetToFormat("\"summary\":\"%s\",", event->GetName());
	jsonString += BString().SetToFormat("\"location\":\"%s\",", event->GetPlace());
	jsonString += BString().SetToFormat("\"description\":\"%s\",", event->GetDescription());
/*  TODO This is optional but this is a wrong cast must be improved better
	EventStatus eventStatus = static_cast<EventStatus>(event->GetStatus());
	BString statusString = kEventStatusToGCalStatus[eventStatus];
	jsonString += BString().SetToFormat("\"status\": \"%s\",", statusString);
*/
	BString start = TimeToRFC3339(event->GetStartDateTime());
	jsonString += BString().SetToFormat("\"start\":{\"dateTime\":\"%s\"},", start.String());

	BString end = TimeToRFC3339(event->GetEndDateTime());
	jsonString += BString().SetToFormat("\"end\":{\"dateTime\":\"%s\"}", end.String());
	jsonString.Append("}");

	BMessage reply;
	if (Requests::Request(url, B_HTTP_POST, headers, NULL, &jsonString, reply)
		== B_ERROR)
		return B_ERROR;

	return B_OK;
}


status_t
EventSync::DeleteEvent(Event* event)
{
	BString endpoint("https://www.googleapis.com/calendar/v3/calendars/primary/events/");
	endpoint.Append(event->GetId());
	endpoint.Append("?access_token=");
	endpoint.Append(fToken);
	BMessage reply;
	if (Requests::Request(endpoint, B_HTTP_DELETE, NULL, NULL, NULL, reply)
		== B_ERROR)
		return B_ERROR;

	return B_OK;
}


status_t
EventSync::SyncWithDatabase()
{
	Event* newEvent;
	Event* event;

	for (int32 i = 0; i < fEvents->CountItems(); i++) {
		newEvent = ((Event*)fEvents->ItemAt(i));
		event = fDBManager->GetEvent(newEvent->GetId());

		if ((event != NULL) && (difftime(newEvent->GetUpdated(), event->GetUpdated()) > 0)) {
			if (fDBManager->UpdateEvent(event, newEvent) == false)
				return B_ERROR;
		}

		else if ((event == NULL) && (fDBManager->AddEvent(newEvent) == false))
			return B_ERROR;
	}

	const char* cancelId;
	for (int32 i = 0; i < fCancelledEvents->CountStrings(); i++) {
		cancelId = ((const char*)fCancelledEvents->StringAt(i));
		event  = fDBManager->GetEvent(cancelId);
		if (event != NULL)  {
			if (fDBManager->RemoveEvent(event) == false)
				return B_ERROR;
		}
	}

	return B_OK;
}


time_t
EventSync::RFC3339ToTime(const char* timeString, EventDateType type)
{
	struct tm timeinfo;
	int	year = 0;
	int	month = 0;
	int	day	= 0;
	int	hour = 0;
	int	minute = 0;
	int	second = 0;
	int zoneHour = 0;
	int zoneMinute = 0;

	if (static_cast<EventDateType>(type) == kEventStartEndDate) {
		sscanf (timeString, "%d-%d-%dT%d:%d:%d%d:%d", &year, &month, &day,
           &hour, &minute, &second, &zoneHour, &zoneMinute);
	}

	else if (static_cast<EventDateType>(type) == kEventUpdateDate) {
		sscanf (timeString, "%d-%d-%dT%d:%d:%d.%*d'Z'", &year, &month, &day,
           &hour, &minute, &second);
	}

	timeinfo.tm_year = year - 1900;
	timeinfo.tm_mon = month - 1;
	timeinfo.tm_mday = day;
	timeinfo.tm_hour = hour;
	timeinfo.tm_min = minute;
	timeinfo.tm_sec = second;

	int offset = (-timezone - (zoneHour* 3600 + zoneMinute * 60));

	return mktime(&timeinfo) + offset;
}


BString
EventSync::TimeToRFC3339(time_t timeT)
{
	BString timeString;
	BDateFormat dateFormat;

	dateFormat.SetDateFormat(B_SHORT_DATE_FORMAT, "y-MM-dd'T'HH:mm:ssXXX");
	dateFormat.Format(timeString, timeT,
		B_SHORT_DATE_FORMAT);
	return timeString;
}
