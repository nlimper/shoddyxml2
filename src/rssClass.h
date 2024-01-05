#include <WiFiClientSecure.h>
#include <shoddyxml2.h>

#define CONNECTION_TIMEOUT_MILLIS 10000

class rssClass : public shoddyxml {
public:
	rssClass();

	char **titleData;
	char **descData;
	int itemNum;

	int getArticles(const char *url, const int maxTitleDataSize, const int maxDescDataSize, const int maxItemNum, const char *rootCA = NULL);
	void clearItemData();

private:
	virtual int getChar();
	virtual void foundXMLDecl();
	virtual void foundPI(char *s);
	virtual void foundSTag(char *s, int numAttributes, attribute_t attributes[]);
	virtual void foundETag(char *s);
	virtual void foundEmptyElemTag(char *s, int numAttributes, attribute_t attributes[]);
	virtual void foundSection(char *s);
	virtual void foundCharacter(char c);
	virtual void foundElement(char *s);

	Client *client;

	int bufTitlePos;
	int bufDescPos;
	int itemDepth;
	int lastTagMatches;
	int maxItemNum;
	int maxTitleDataSize;
	int maxDescDataSize;

	void resetStatus();
	void skipHeaders();
};
