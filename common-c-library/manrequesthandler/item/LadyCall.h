/*
 * LadyCall.h
 *
 *  Created on: 2015-3-2
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef LADYCALL_H_
#define LADYCALL_H_

#include <string>
using namespace std;


#include <json/json/json.h>

#include "../RequestLadyDefine.h"

class LadyCall {
public:
	void Parse(Json::Value root) {
		if( root.isObject() ) {
			if( root[LADY_WOMAN_ID].isString() ) {
				womanid = root[LADY_WOMAN_ID].asString();
			}

			if( root[LADY_LOVECALLID].isString() ) {
				lovecallid = root[LADY_LOVECALLID].asString();
			}

			if( root[LADY_LC_CENTERNUMBER].isString() ) {
				lc_centernumber = root[LADY_LC_CENTERNUMBER].asString();
			}
		}
	}

	LadyCall() {		womanid = "";
		lovecallid = "";
		lc_centernumber = "";
	}
	virtual ~LadyCall() {

	}

	/**
	 * 4.7.获取女士Direct Call TokenID成功回调
	 * @param womanid				被呼叫女士ID
	 * @param lovecallid			被呼叫女士TokenID
	 * @param lc_centernumber		call center number
	 */

	string womanid;
	string lovecallid;
	string lc_centernumber;

};

#endif /* LADYCALL_H_ */
