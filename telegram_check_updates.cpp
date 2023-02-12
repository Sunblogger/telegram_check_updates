/*
 telegram_check_updates.cpp
 Version: 0.4
 Date: 05.02.2023
  
 * Copyright 2023 
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include <filesystem>
#include <unistd.h>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <stopfile.hpp>			// include-file for handling stop-file
#include <logfile.hpp>			// include-file for handling log-file

// #define DEVSTAGE	// we are in development-stage: we print some more messages on screen than desired in production release

const std::string version_of_program = "0.3";

unsigned int number_of_cycles;	// how many times will we repeat to get updates?
unsigned int wait_time_cycles;	// waittime before we get next update
std::string messages_save_path;	// path where we save the received messages

//**********************************************************************************************************
// 	This function prints a little help on the screen if we did not get desired command-line-parameters
//**********************************************************************************************************
void print_help() {
	std::cout << "Wrong number of arguments for this program!" << std::endl;
	std::cout << "First argument: token (bot-id + token, example: 123796789:ABCDGefGhiJKV_fhuedf8742lllJzzpLwpk)" << std::endl;
	std::cout << "Second argument: number of cycles to run (1 or greater)" << std::endl;
	std::cout << "Third argument: waittime between cycle expressed in 0.1 seconds" << std::endl;
	std::cout << "Optional fourth argument: path were to save messages (must end with /)" << std::endl;
	std::cout << "If leave fourth argument than saving path for messages will be current directory." << std::endl;
	std::cout << "If number of cycles = 1 than waittime will be ignored." << std::endl;	
	std::cout << "If number of cycles = 0 than we will repeat until we are stopped by stop-file." << std::endl;	
}	// end of print_help

//****************************************************************************************************************************************
// class to work on the received url-data specific for telegram-messages
//****************************************************************************************************************************************
class telegram_messageclass {
public:
std::string curldata;	// the data which comes from the call of curl
std::string bot_adress_updates;	// the https-adress of the bot to get updates
static size_t write_curl_data_to_buffer (void * content, size_t size, size_t members, void *urldata);	
void get_updates();		// calls the Telegram bot to answer with updates
void get_message_in_url_data();	// get the messages from url-string	
void clear_updates();			// clear updates so that we do not get same updates again
void set_logfile(logfile_class *logfileclass) { logclass = logfileclass;};	// we tell our class which object can be used for logging. This can be used to switch to another logfile
explicit telegram_messageclass(logfile_class *logfileclass) { logclass = logfileclass;};	// this constructor forces us to use logging with logfile_class
	
private:
logfile_class *logclass = nullptr;	// a pointer to an object of type logfile_class; we initialize with nullptr so that we can check if we have a logfile or not
const std::string no_content_result = ",\"result\":[]}"; // when we have this string then we know, that we do not have any messages in url-data
unsigned int latest_update_id;	// the latest update_id: this id will be used to clear all updates

struct message_from_struct {	// struct to hold data from whom we received the telegram-message
	unsigned int id;			// the unique id of the chat
	bool is_bot;				// can contain "true" or "false"
	std::string first_name; 	
	std::string last_name;
	std::string username;
	std::string language_code;	// example: "de"
};

struct message_chat_struct {	// struct to hold chat data
	unsigned int id;			// the unique id of the chat
	std::string first_name; 
	std::string last_name;
	std::string username;
	std::string type; 			// example: "private"	 
};

struct message_message_struct {		// struct to hold a complete message
	unsigned int message_id, date;
	std::string text;
	message_from_struct from;
	message_chat_struct chat;
};

struct message_update_struct {		// struct to hold an update
	unsigned int update_id;
	message_message_struct message;
};

std::string message_text;		// the text of the message 
std::size_t tempfound;
unsigned int message_counter;	// counts the # of messages we have in url
	
};	// end of class telegram_message

//**********************************************************************************************************
// 	This function writes the received data, we received with a call of curl_easy_perform, to a own buffer
//	Input: 	content: a pointer to a buffer which holds the data curl can deliver to us
//			size: the length of the content
//			members: usually this value is 1
//			urldata: a pointer to an onw buffer to save the url-data
//	return-value: the complete length of the data
//**********************************************************************************************************
size_t telegram_messageclass::write_curl_data_to_buffer (void * content, size_t size, size_t members, void *urldata) {
	((std::string*) urldata)->assign((char*)content, size * members);
	return size * members;
}	// end of write_curl_data_to_buffer


//****************************************************************************************************************************************
// This method uses curl to get updates from Telegram-bot
//****************************************************************************************************************************************
void telegram_messageclass::get_updates() {
CURL *curl;				// pointer to CURL-object
CURLcode curl_result;	// returncode of calling CURL-functions

curl = curl_easy_init();	// init curl-functions
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, bot_adress_updates.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_data_to_buffer);	// we tell curl which function is to be used to write the url-data to our own buffer
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&curldata);	// we tell curl to which buffer the received data have to be written
		curl_result = curl_easy_perform(curl);	// we execute 1 call of curl
		if(curl_result != CURLE_OK) { 
			if (logclass != nullptr) { logclass->addmessage(logfile_class::logentrytype::error, "Called URL is not valid for Telegram! curl_easy_perform() failed!");}
			#ifdef DEVSTAGE
			std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(curl_result) << std::endl;
			#endif
		}
		curl_easy_cleanup(curl);
	}
}	// end of telegram_messageclass::get_updates

//****************************************************************************************************************************************
// This method extracts the message from telegram-chat and saves the message in a file
//****************************************************************************************************************************************
void telegram_messageclass::get_message_in_url_data() {

Json::CharReaderBuilder jsonreaderbuilder;
Json::Value jsonvalue;
Json::Value result, message, from, chat, date, text;
JSONCPP_STRING parseerror;	

std::string tempstring;
std::string logentry;
message_counter = 0;	// counts the number of messages we have received
message_update_struct message_update;	// one object of a struct containing the whole message	
std::ofstream message_file;	// the file which will save the message on the disk
std::chrono::time_point<std::chrono::system_clock> systemtime;		// the current point of time
double epoch_counter;
std::string counter_string;

std::time_t timepoint;
std::string timestamp;
std::string timestamp_string;

tempfound = curldata.find("{\"ok\":true");
if (tempfound != std::string::npos) {	// we have a positive result from Telegram
	tempfound = curldata.find(no_content_result);
	if (tempfound == std::string::npos) {	// we do not have an empty data result: there are some data we can extract from the received url-data
		#ifdef DEVSTAGE
			std::cout << "We have some data received." << std::endl;
		#endif
		const std::unique_ptr<Json::CharReader> jsonreader(jsonreaderbuilder.newCharReader());
		jsonreader->parse(curldata.c_str(), curldata.c_str() + curldata.length(), &jsonvalue, &parseerror);
		result = jsonvalue["result"];
		for (unsigned int resultcounter = 0; resultcounter < result.size(); ++resultcounter) { // we step through all messages we have
			message = result[resultcounter];	// we copy 1 message JSON-object to our json-variable	
			from = message["message"]["from"];	// we copy from-JSON-object to our json-variable from
			chat = message["message"]["chat"];	// we copy chat-JSON-object to our json-variable chat
			message_update.update_id = message["update_id"].asUInt();	
			latest_update_id = message_update.update_id;	// everytime we find an update_id we set the latest_update_id to this value
			message_update.message.date = message["message"]["date"].asUInt();
			message_update.message.text = message["message"]["text"].asString(); 
			message_update.message.from.id = from["id"].asUInt();
			message_update.message.from.is_bot = from["is_bot"].asBool();
			message_update.message.from.first_name = from["first_name"].asString();
			message_update.message.from.last_name = from["last_name"].asString();
			message_update.message.from.username = from["username"].asString();
			message_update.message.from.language_code = from["language_code"].asString();
			
			if (chat["id"].isUInt() == true) { 	// if chat-id is a positive number:
				message_update.message.chat.id = chat["id"].asUInt(); 
				message_update.message.chat.first_name = chat["first_name"].asString();
				message_update.message.chat.last_name = chat["last_name"].asString();
				message_update.message.chat.username = chat["username"].asString();
				systemtime = std::chrono::system_clock::now();
				timepoint = std::chrono::system_clock::to_time_t(systemtime);	// convert current systemtime to a value that represents date + time
				timestamp.resize(25);	// we set timestamp to 25 chars 
				std::strftime(&timestamp[0], timestamp.size(), "%Y%m%d_%H%M%S_", std::localtime(&timepoint));	// we get the timestamp in format dd.mm.yyyy_hhminminsecsec
				epoch_counter = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				counter_string = std::to_string(epoch_counter);		// we convert to string
				counter_string.erase(counter_string.length() - 7, counter_string.length());	// we delete ".000000"
				counter_string.erase(0, counter_string.length() - 4);	// we keep only last 4 digits
				timestamp_string.assign(&timestamp[0], 16);		// we copy only 16 chars
				timestamp_string.append(counter_string);	// we append counter to get a unique-string
				timestamp_string.append(".txt");			// we append suffix ".txt"
				tempstring = messages_save_path;
				tempstring.append(timestamp_string);
				message_file.open(tempstring, std::ios_base::app);
				if (message_file.is_open() == true) {
					message_file << "Name: message_from_chat" << std::endl << "Chat-ID: " << message_update.message.chat.id << std::endl << "Message: " << message_update.message.text << std::endl;
					message_file.close();
					if (logclass != nullptr) { 
						logentry.assign("We saved received message in file ");
						logentry.append(tempstring);
						logclass->addmessage(logfile_class::logentrytype::info, logentry);
					}
					
				} else {	// we can not open message-file for writing the message
					if (logclass != nullptr) { 
						logentry.assign("We can not open message-file for writing: ");
						logentry.append(messages_save_path);
						logclass->addmessage(logfile_class::logentrytype::error, logentry);}
				}
			} else { message_update.message.chat.id = 0; }	// we expect to get positive values and not negative ones
		}	// end of for-loop
		clear_updates();
	} else {	// there are no data for us
		if (logclass != nullptr) { logclass->addmessage(logfile_class::logentrytype::info, "There are no data for us.");}
	}
} else {	// we did not get "{"ok":true" from Telegram; we will log this:
	if (logclass != nullptr) { 
			tempstring = "We did not get '{\"ok\":true' from Telegram. This is result: ";
			tempstring.append(curldata);
			logclass->addmessage(logfile_class::logentrytype::error, tempstring);
	}
}	// 	 
}; 	// end of telegram_messageclass::get_message_from_url_data

//****************************************************************************************************************************************
// we clear the updates by sending a request to set an update.
//****************************************************************************************************************************************
void telegram_messageclass::clear_updates() {
CURL *curl;				// pointer to CURL-object
CURLcode curl_result;	// returncode of calling CURL-functions
std::string clear_update = bot_adress_updates;
unsigned int tempint = latest_update_id + 1 ;

clear_update.append("?offset=");	// we add "?offset=" at the end of the adress
clear_update += std::to_string(tempint);	// we add offset so that we clear all updates

curl = curl_easy_init();	// init curl-functions
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, clear_update.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_data_to_buffer);	// we tell curl which function is to be used to write the url-data to our own buffer
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&curldata);	// we tell curl to which buffer the received data have to be written
		curl_result = curl_easy_perform(curl);	// we execute 1 call of curl
		if(curl_result != CURLE_OK) { 
			if (nullptr != logclass) logclass->addmessage(logfile_class::logentrytype::error, "Called URL is not valid for Telegram! curl_easy_perform() failed!");
			#ifdef DEVSTAGE
			std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(curl_result) << std::endl;
			#endif
		}
		curl_easy_cleanup(curl);
	}
}	// end of telegram_messageclass::clear_updates

//****************************************************************************************************************************************
// we check if paramters for our program are correct or not
// returncode = 0: OK
// returcode = 1: not ok
//****************************************************************************************************************************************
unsigned int check_program_parameters(const int argument_counter, const char * arguments[], std::string *urlstring) {
std::string number_of_cycles_string;
std::string wait_time_cycles_string;

if ((4 == argument_counter) || (5 == argument_counter)) {	// if we have 4 or 5 parameters for the program: we check now if they are valid
	*urlstring = "https://api.telegram.org/bot";
	urlstring->append(arguments[1]);
	urlstring->append("/getUpdates");
	number_of_cycles_string = arguments[2];
	wait_time_cycles_string = arguments[3];
	try {	// we need integers of given parameter #2
		number_of_cycles = std::stoi(number_of_cycles_string, nullptr, 10);
	} catch (const std::invalid_argument &except) {
		#ifdef DEVSTAGE
		std::cout << except.what() << std::endl;
		#endif
		std::cout << "Invalid parameter given for number of cycles: " << number_of_cycles_string << std::endl;
		return 1;
	}
	
	try {	// we need integers of given parameter #3
		wait_time_cycles = std::stoi(wait_time_cycles_string, nullptr, 10);
	} catch (const std::invalid_argument &except) {
		#ifdef DEVSTAGE
		std::cout << except.what() << std::endl;
		#endif
		std::cout << "Invalid parameter given for number of waittime: " << number_of_cycles_string << std::endl;
		return 1;
	}
	if (5 == argument_counter) messages_save_path.assign(arguments[4]); // if we have 4 arguments: the last argument is the path for saving the data from telegram
	else messages_save_path.assign("./"); 	// if we have 3 arguments the path for saving the files is same path as the program is running in
} else { 	// we did not get correct # of parameters, we print a smal help:
	print_help();
	return 1;
}

return 0;
}	// end of check_program_parameters

//****************************************************************************************************************************************
// This is main
//****************************************************************************************************************************************
int main(int argc, char *argv[])
{
std::string url_string;	// the adress we call with curl. We start with this string but we will append parameters from user		
std::string logentry;
unsigned int cycles_counter;
logfile_class logfile((std::string)(argv[0] + std::string(".log")), 200, true);	// we create one object of logging-class: maximum-size = 200 kByte, logging = true
stop_file_class stop_file((std::string)(argv[0]) + std::string(".stop"));	// one object of class to deal with the stop-file. We simply take the name of the program with complete path and add ".stop" as suffix
telegram_messageclass telegram_message(&logfile);	// one object to get and save the telegram data we need

std::cout << "Telegram Check Updates for message2action, version " << version_of_program << std::endl;
std::cout << "Filename for stopping program: " << stop_file.get_stopfile_name() << std::endl;

logfile.addmessage(logfile_class::logentrytype::info, "Program started.");
if (0 == check_program_parameters(argc, (const char **)argv, &url_string)) {	// if we have valid parameters we will continue
	std::cout << "Received messages will be saved in path " << messages_save_path << std::endl;
	telegram_message.bot_adress_updates = url_string;
	if (number_of_cycles == 0) { 
		std::cout << "Program will run until stop-file is detected." << std::endl; 
		logfile.addmessage(logfile_class::logentrytype::info, "Program will run until stop-file is detected.");
		do {	// we repeat until end of time or until we detect a stop-file
			telegram_message.get_updates();
			telegram_message.get_message_in_url_data();	// we extract the message which we received out of the received url-data
			logfile.addmessage(logfile_class::logentrytype::info, "We sleep now.");
			std::this_thread::sleep_for(std::chrono::milliseconds(100*wait_time_cycles));
		} while (stop_file.check_stop_file() == stop_file.returncode::NO_STOP_FILE); 
		logfile.addmessage(logfile_class::logentrytype::info, "Program finished.");
		std::cout << "Program finished." << std::endl;   
		return 0;
	} else { // we do not have a 0 for number of cycles
		std::cout << "Program will run with " << number_of_cycles << " loops and " << 100*wait_time_cycles << " millliseconds waittime." << std::endl; 
		logentry = "Program will run with ";
		logentry.append(std::to_string(number_of_cycles));
		logentry.append(" loops and wait time ");
		logentry.append(std::to_string(100*wait_time_cycles));
		logentry.append(" milliseconds.");
		logfile.addmessage(logfile_class::logentrytype::info, logentry);
		
		for (cycles_counter = 0; cycles_counter < number_of_cycles; ++cycles_counter) {
			if (stop_file.check_stop_file() != stop_file.returncode::NO_STOP_FILE) { 	// if we have a stop-file then we have to stop our program
				cycles_counter = number_of_cycles; 
				logfile.addmessage(logfile_class::logentrytype::info, "We have detected a stop-file.");
				#ifdef devstage
				std::cout << "We have detected a stop-file." << std::endl; 
				#endif 
			} else {	// if we have not found a valid stop-file we continue
				telegram_message.get_updates();
				telegram_message.get_message_in_url_data();	// we extract the message which we received out of the received url-data
			}
			if (cycles_counter < number_of_cycles -1) {	// we skip last wait cylce
				logfile.addmessage(logfile_class::logentrytype::info, "We sleep now.");
				std::this_thread::sleep_for(std::chrono::milliseconds(100*wait_time_cycles));
			}	
		}	// end of for-loop	
		logfile.addmessage(logfile_class::logentrytype::info, "Program finished.");
		std::cout << "Program finished." << std::endl;   
		return 0;
	}
}

}	// end of main
