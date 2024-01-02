#include "rssClass.h"
#include <string.h>

rssClass::rssClass() {
}

int rssClass::getArticles(const char *url, const int maxTitleDataSize, const int maxDescDataSize, const int maxItemNum, const char *rootCA) {
	int c;
	int port = 0;
	char protocol[8], server[32];

	clearItemData();
	resetStatus();

	this->maxItemNum = maxItemNum;

	titleData = new char *[maxItemNum];
	for (int i = 0; i < maxItemNum; i++) {
		titleData[i] = new char[maxTitleDataSize];
	}
	this->maxTitleDataSize = maxTitleDataSize;

	descData = new char *[maxItemNum];
	for (int i = 0; i < maxItemNum; i++) {
		descData[i] = new char[maxDescDataSize];
	}
	this->maxDescDataSize = maxDescDataSize;

	sscanf(url, "%7[^:]://%31[^:/]:%6d/", protocol, server, &port);

	if (strcmp(protocol, "https") == 0) {
		WiFiClientSecure *c = new WiFiClientSecure();
		if (rootCA) {
			c->setCACert(rootCA);
		} else {
			c->setInsecure();
		}
		client = (Client *)c;
		if (!port) {
			port = 443;
		}
	} else if (strcmp(protocol, "http") == 0) {
		client = new WiFiClient();
		if (!port) {
			port = 80;
		}
	} else {
		return 0;
	}

	if (client->connect(server, port) == 1) {
		client->print("GET ");
		client->print(url);
		client->println(" HTTP/1.1");
		client->print("Host: ");
		client->println(server);
		client->println("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4577.82 Safari/537.36");
		client->println("Connection: close");
		client->println();
		client->flush();

		skipHeaders();
		init(1);
		while ((c = getChar()) != EOF) {
			parse(c);
		}
		client->stop();
	}

	delete client;
	client = NULL;
	return itemNum;
}

void rssClass::clearItemData() {
	if (titleData) {
		for (int i = 0; i < maxItemNum; i++) {
			delete titleData[i];
		}
		delete[] titleData;
		titleData = (char **)NULL;
	}
	if (descData) {
		for (int i = 0; i < maxItemNum; i++) {
			delete descData[i];
		}
		delete[] descData;
		descData = (char **)NULL;
	}
}

int rssClass::getChar() {
	static unsigned long lastRead = 0;
	if (client->connected()) {
		if (client->available()) {
			lastRead = millis();
			return client->read();
		} else {
			if ((millis() - lastRead) > CONNECTION_TIMEOUT_MILLIS) {
				return EOF;
			} else {
				return 0;
			}
		}
	} else {
		return EOF;
	}
}

void rssClass::foundXMLDecl() {
}

void rssClass::foundPI(char *s) {
}

void rssClass::foundSTag(char *s, int numAttributes, attribute_t attributes[]) {
	if (strcmp(s, "item") == 0) {
		itemDepth++;
	}

	if (strcmp(s, "title") == 0) {
		lastTagMatches = 1;
	} else if (strcmp(s, "description") == 0) {
		lastTagMatches = 2;
	} else {
		lastTagMatches = 0;
	}
}

void rssClass::foundETag(char *s) {
	if (itemNum < maxItemNum) {
		if ((itemDepth == 1) && (strcmp(s, "title") == 0)) {
			titleData[itemNum++][bufTitlePos] = '\0';
			bufTitlePos = 0;
		}
		if ((itemDepth == 2) && (strcmp(s, "description") == 0)) {
			descData[itemNum++][bufDescPos] = '\0';
			bufDescPos = 0;
		}
		if (strcmp(s, "item") == 0) {
			itemDepth--;
		}
	}
}

void rssClass::foundEmptyElemTag(char *s, int numAttributes, attribute_t attributes[]) {
}

void rssClass::foundSection(char *s) {
}

void rssClass::foundCharacter(char c) {
	if (itemDepth == 1) {
		if (lastTagMatches == 1 && bufTitlePos < (maxTitleDataSize - 1)) {
			titleData[itemNum][bufTitlePos++] = c;
		}
		if (lastTagMatches == 2 && bufDescPos < (maxDescDataSize - 1)) {
			descData[itemNum][bufDescPos++] = c;
		}
	}
}

void rssClass::foundElement(char *s) {
}

void rssClass::resetStatus() {
	itemDepth = 0;
	lastTagMatches = 0;
	itemNum = 0;
	bufTitlePos = 0;
	bufDescPos = 0;
}

void rssClass::skipHeaders() {
	while (client->connected()) {
		String line = client->readStringUntil('\n');
		if (line == "\r") {
			break;
		}
	}
}
