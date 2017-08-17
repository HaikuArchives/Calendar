/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#include <sstream>
#include <time.h>

#include <Button.h>
#include <DateFormat.h>
#include <Handler.h>
#include <LayoutBuilder.h>
#include <List.h>
#include <Key.h>
#include <KeyStore.h>
#include <Roster.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>

#include "Category.h"
#include "Event.h"
#include "EventSync.h"
#include "Requests.h"
#include "SQLiteManager.h"


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
		LoginDialog(EventSync* sync)
			:
			BWindow(BRect(),"Authorization", B_TITLED_WINDOW,
				B_NOT_RESIZABLE | B_NOT_ZOOMABLE),
				fSync(sync)
		{
			BView* fMainView = new BView("MainView", B_WILL_DRAW);
			fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

			fLabel = new BStringView("AuthCodeLabel",
				"Google Calendar API Authorization");

			BFont font;
			fLabel->GetFont(&font);
			font.SetSize(font.Size() * 1.4);
			fLabel->SetFont(&font, B_FONT_ALL);

			fAuthCodeText = new BTextControl("AuthCodeText", NULL,
				"Enter the Authorization Code obtained here.", NULL);

			fLogin = new BButton(NULL, "OK", new BMessage(kAuthCode));
			BMessage* fAuthMessage = new BMessage(kAuthorizationCode);

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
					fAuthString = fAuthCodeText->Text();
					fAuthMessage->AddString("auth", fAuthString);
					fSync->NextStep(fAuthMessage);
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
			fSync->NextStep(fAuthMessage);
			return true;
		}

		static const int kAuthCode = 1000;

		BStringView*		fLabel;
		BTextControl*		fAuthCodeText;
		BButton*			fLogin;
		BString				fAuthString;
		EventSync*			fSync;
		BMessage*			fAuthMessage;
};


EventSync::EventSync(BHandler* handler)
{
	fDBManager = new SQLiteManager();
	fEvents = new BList();
	fHandler = handler;
}


EventSync::~EventSync()
{
	delete fDBManager;
}


status_t
EventSync::LoadToken()
{
	BPasswordKey key;
	BKeyStore keyStore;
	if(keyStore.GetKey("Calendar", B_KEY_TYPE_PASSWORD, "refresh_token", key) == B_OK) {
		fRefreshToken = key.Password();
		return B_OK;
	}
	return B_ERROR;
}


bool
EventSync::Login()
{
	if (LoadToken() == B_OK) {
		BHttpForm* form = new BHttpForm();
		form->AddString("refresh_token", fRefreshToken);
		form->AddString("client_id", CLIENT_ID);
		form->AddString("client_secret", CLIENT_SECRET);
		form->AddString("grant_type", "refresh_token");
		form->SetFormType(B_HTTP_FORM_URL_ENCODED);
		BString url("https://www.googleapis.com/oauth2/v4/token");

		BMessage refreshJson;
		if (Requests::Request(url, B_HTTP_GET, NULL, form, NULL, refreshJson, false)
				== B_ERROR)
			return B_ERROR;


		fToken = BString(refreshJson.GetString("access_token", "NOT_FOUND"));
		if (fToken.Compare("NOT_FOUND") == 0)
		{
			BPasswordKey key;
			BKeyStore keyStore;
			keyStore.GetKey("Calendar", B_KEY_TYPE_PASSWORD, "refresh_token", key);
			keyStore.RemoveKey("Calendar", key);
		}

		else
		{
			GetEvents();
			return true;
		}
	}

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
	LoginDialog* loginWindow = new LoginDialog(this);
	loginWindow->Show();
}


void
EventSync::NextStep(BMessage* authMessage)
{
	BString code;
	if (authMessage->FindString("auth", &code) != B_OK) {

	}

	BHttpForm* form = new BHttpForm();
	form->AddString("code",code);
	form->AddString("client_id",CLIENT_ID);
	form->AddString("client_secret",CLIENT_SECRET);
	form->AddString("grant_type","authorization_code");
	form->AddString("redirect_uri",REDIRECT_URI);
	form->SetFormType(B_HTTP_FORM_URL_ENCODED);

	BString oauth2("https://www.googleapis.com/oauth2/v3/token");
	BMessage tokenJson;
	status_t status;
	status = Requests::Request(oauth2, B_HTTP_GET, NULL, form, NULL, tokenJson, false);

	fToken = BString(tokenJson.GetString("access_token","NOT_FOUND"));
	fRefreshToken = BString(tokenJson.GetString("refresh_token","NOT_FOUND"));

	BPasswordKey key(fRefreshToken, B_KEY_PURPOSE_WEB, "refresh_token");
	BKeyStore keyStore;
	keyStore.AddKeyring("Calendar");
	keyStore.AddKey("Calendar", key);
}


status_t
EventSync::LoadSyncToken()
{
	BPasswordKey key;
	BKeyStore keyStore;
	if(keyStore.GetKey("Calendar", B_KEY_TYPE_PASSWORD, "nextSyncToken", key) == B_OK) {
		fLastSyncToken = key.Password();
		return B_OK;
	}
	fLastSyncToken.Append("NOT_FOUND");
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
	BString currentNextSyncToken;
	BString nextSyncToken;

	do {
		if (!eventJson.IsEmpty())
			eventJson.MakeEmpty();

		if (Requests::Request(url, B_HTTP_GET, headers, NULL, NULL, eventJson, false)
				== B_ERROR)
			return B_ERROR;

		if (ParseEvent(&eventJson) == B_ERROR);
			return B_ERROR;

		nextPageToken = BString(eventJson.GetString("nextPageToken", "NOT_FOUND"));
		url.Append("?pageToken=");
		url.Append(nextPageToken);

	} while (nextPageToken.Compare("NOT_FOUND") != 0);

	currentNextSyncToken = BString(eventJson.GetString("nextSyncToken", "NOT_FOUND"));

	if (currentNextSyncToken.Compare("NOT_FOUND") != 0)
		nextSyncToken = currentNextSyncToken;
	else
		nextSyncToken = fLastSyncToken;

	BPasswordKey key(nextSyncToken, B_KEY_PURPOSE_WEB, "nextSyncToken");
	BKeyStore keyStore;
	keyStore.AddKey("Calendar", key);

	if (currentNextSyncToken.Compare("NOT_FOUND") == 0) {
		fprintf(stderr, "Error: nextSyncToken not found in API response.\n");
		return B_ERROR;
	}

	Event* event;
	for (int32 i = 0; i < fEvents->CountItems(); i++) {
		event = ((Event*)fEvents->ItemAt(i));
		std::cout<<"\n"<<event->GetName()<<"\n";
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
		fprintf(stderr, "Error: 0 items found in API response.\n");
		return B_ERROR;
	}

	for (int32 currentEvent = 0; currentEvent < eventsCount; currentEvent++) {
		std::ostringstream ss;
		ss << currentEvent;
		BMessage event;
		if (items.FindMessage(ss.str().c_str(), &event) != B_OK) {
			fprintf(stderr, "Error: Item %d found in API response.\n",
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

		bool status;
		bool notified;

		event.FindString("id", &id);
		event.FindString("summary", &name);
		event.FindString("status", &eventStatus);
		place = BString(event.GetString("location", ""));
		description = BString(event.GetString("description", ""));

		if (eventStatus == BString(kEventStatusToGCalStatus[kConfirmedEvent])
			|| eventStatus == BString(kEventStatusToGCalStatus[kTentativeEvent]))
			status = kConfirmedEvent;
		else if (eventStatus == BString(kEventStatusToGCalStatus[kTentativeEvent]))
			status = kCancelledEvent;

		event.FindString("updated", &updatedString);
		updated = RFC3339ToTime(updatedString, kEventUpdateDate);

		// TODO:
		// Check for events having only start date (all day event).
		// Check whether reminder option is set on or off in GCal event.
		// Incorporate color IDs of GCal events into Calendar.
		// Add support for multiple calendar, treat them as a category maybe?

		BMessage start;
		if (event.FindMessage("start", &start) != B_OK) {
			fprintf(stderr, "Error: StartTime not found in API response.\n");
			return B_ERROR;
		}
		start.FindString("dateTime", &startString);
		startDateTime = RFC3339ToTime(startString, kEventStartEndDate);

		BMessage end;
		if (event.FindMessage("end", &end) != B_OK) {
			fprintf(stderr, "Error: EndTime not found in API response.\n");
			return B_ERROR;
		}

		end.FindString("dateTime", &endString);
		endDateTime = RFC3339ToTime(endString, kEventStartEndDate);

		notified = (difftime(startDateTime, BDateTime::CurrentDateTime(B_LOCAL_TIME).Time_t()) < 0) ? true : false;

		BList* categoryList = fDBManager->GetAllCategories();
		Category* category;
		for (int32 i = 0; i < categoryList->CountItems(); i++) {
			category = ((Category*)categoryList->ItemAt(i));
			if (BString(category->GetName()) == BString("Default"))
				break;
		}

		Category* newCategory = new Category(*category);

		Event* newEvent = new Event(name, place, description, false,
			startDateTime, endDateTime, newCategory, notified, updated,
			status, id);

		fEvents->AddItem(newEvent);
	}

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

	EventStatus eventStatus = static_cast<EventStatus>(event->GetStatus());
	BString statusString = kEventStatusToGCalStatus[eventStatus];
	jsonString += BString().SetToFormat("\"status\": \"%s\",", statusString);

	BString start = TimeToRFC3339(event->GetStartDateTime());
	jsonString += BString().SetToFormat("\"start\":{\"dateTime\":\"%s\"},", start.String());

	BString end = TimeToRFC3339(event->GetEndDateTime());
	jsonString += BString().SetToFormat("\"end\":{\"dateTime\":\"%s\"}", end.String());
	jsonString.Append("}");

	BMessage reply;
	if (Requests::Request(url, B_HTTP_POST, headers, NULL, &jsonString, reply, true)
		== B_ERROR)
		return B_ERROR;
}


status_t
EventSync::DeleteEvent(Event* event)
{
	BString endpoint("https://www.googleapis.com/calendar/v3/calendars/primary/events/");
	endpoint.Append(event->GetId());
	endpoint.Append("?access_token=");
	endpoint.Append(fToken);
	BMessage reply;
	if (Requests::Request(endpoint, B_HTTP_DELETE, NULL, NULL, NULL, reply, true)
		== B_ERROR)
		return B_ERROR;
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
		sscanf (timeString, "%d-%d-%dT%d:%d:%d.%d'Z'", &year, &month, &day,
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
