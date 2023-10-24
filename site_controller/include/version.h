/*
 * Asset.h
 *
 *  Created on: 2021-02-03
 *      Author: Anirudh Mulukutla
 */

#ifndef VERSION_H_
#define VERSION_H_

#include <string>

class Version {
public:
    Version();
    virtual ~Version();

    void init(void);  // initialization, call after object creation

    char* get_build(void);
    char* get_commit(void);
    char* get_tag(void);

private:
    char* build;
    char* commit;
    char* tag;

    char* extract(char* info, char* start, const char* end);
};

#endif /* VERSION_H_ */
