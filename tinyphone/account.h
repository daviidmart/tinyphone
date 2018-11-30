#pragma once

#ifndef ACCOUNT_HEADER_FILE_H
#define ACCOUNT_HEADER_FILE_H

#include <pjsua2.hpp>
#include <iostream>
#include <boost/foreach.hpp>

using namespace std;
using namespace pj;

class SIPAccount;

class SIPCall : public Call
{
private:
	SIPAccount *myAcc;

public:
	SIPCall(Account &acc, int call_id = PJSUA_INVALID_ID)
		: Call(acc, call_id)
	{
		myAcc = (SIPAccount *)&acc;
	}

	SIPAccount* getAccount() {
		return myAcc;
	}

	virtual void onCallState(OnCallStateParam &prm);
	virtual void onCallMediaState(OnCallMediaStateParam &prm);
	virtual bool HoldCall();
	virtual bool UnHoldCall();

};

class SIPAccount : public Account
{
	std::string account_name;
	std::string domain;

public:
	std::vector<SIPCall *> calls;

	SIPAccount(std::string name)
	{
		account_name = name;
	}

	~SIPAccount()
	{
		std::cout << "*** Account is being deleted: No of calls=" << calls.size() << std::endl;
	}

	std::string Name() {
		return account_name;
	}

	void removeCall(SIPCall *call)
	{
		for (auto it = calls.begin(); it != calls.end(); ++it) {
			if (*it == call) {
				calls.erase(it);
				break;
			}
		}
	}

	std::vector<SIPCall *> getCalls() {
		return calls;
	}

	virtual void onRegState(OnRegStateParam &prm)
	{
		AccountInfo ai = getInfo();
		std::cout << (ai.regIsActive ? "*** Register: code=" : "*** Unregister: code=")
			<< prm.code << std::endl;
	}

	void HoldAllCalls() {
		BOOST_FOREACH(SIPCall* c, calls) {
			c->HoldCall();
		}
	}

	virtual void onCallEstablished(SIPCall *call) {
		//hold all the other calls
		BOOST_FOREACH(SIPCall* c, calls) {
			if (c != call) {
				c->HoldCall();
			}
		}
	}

	virtual void onIncomingCall(OnIncomingCallParam &iprm)
	{
		SIPCall *call = new SIPCall(*this, iprm.callId);
		CallInfo ci = call->getInfo();
		CallOpParam prm;

		std::cout << "*** Incoming Call: " << ci.remoteUri << " ["
			<< ci.stateText << "]" << std::endl;

		calls.push_back(call);
		prm.statusCode = pjsip_status_code::PJSIP_SC_OK;
		call->answer(prm);
		onCallEstablished(call);
	}

};

void SIPCall::onCallState(OnCallStateParam &prm)
{
	PJ_UNUSED_ARG(prm);
	CallInfo ci = getInfo();
	std::cout << "*** Call: " << ci.remoteUri << " [" << ci.stateText
		<< "]" << std::endl;

	switch (ci.state) {
	case PJSIP_INV_STATE_DISCONNECTED:
		myAcc->removeCall(this);
		/* Delete the call */
		delete this;
		break;
	case PJSIP_INV_STATE_CONFIRMED:
		break;
	default:
		break;
	}
}


void SIPCall::onCallMediaState(OnCallMediaStateParam &prm)
{
	CallInfo ci = getInfo();
	// Iterate all the call medias
	for (unsigned i = 0; i < ci.media.size(); i++) {
		if (ci.media[i].type == PJMEDIA_TYPE_AUDIO && getMedia(i)) {
			AudioMedia *aud_med = (AudioMedia *)getMedia(i);
			// Connect the call audio media to sound device
			AudDevManager& mgr = Endpoint::instance().audDevManager();
			aud_med->startTransmit(mgr.getPlaybackDevMedia());
			mgr.getCaptureDevMedia().startTransmit(*aud_med);
		}
	}
}


bool SIPCall::HoldCall() {

	auto call_info = getInfo();
	if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
		//must have a local media
		if (call_info.media.size() > 0) {
			auto current_media = call_info.media.front();
			if (current_media.status != PJSUA_CALL_MEDIA_LOCAL_HOLD && current_media.status != PJSUA_CALL_MEDIA_NONE) {
				CallOpParam prm;
				prm.options = PJSUA_CALL_UPDATE_CONTACT;
				setHold(prm);
				return true;
			}
			else
				PJ_LOG(3, (THIS_FILE, "Hold Failed, already on hold maybe?"));
		}
		else
			PJ_LOG(3, (THIS_FILE, "Hold Failed, Call Doesn't have any media"));
	}
	else
		PJ_LOG(3, (THIS_FILE, "Hold Failed, Call Not in Confirmed State"));
	return false;
}

bool SIPCall::UnHoldCall() {

	auto call_info = getInfo();
	if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
		if (call_info.media.size() > 0) {
			auto current_media = call_info.media.front();
			if (current_media.status == PJSUA_CALL_MEDIA_LOCAL_HOLD || current_media.status == PJSUA_CALL_MEDIA_NONE) {
				CallOpParam prm(true);
				prm.opt.audioCount = 1;
				prm.opt.videoCount = 0;
				prm.opt.flag |= PJSUA_CALL_UNHOLD;
				reinvite(prm);
				return true;
			}
			else
				PJ_LOG(3, (THIS_FILE, "UnHold Failed, already active maybe?"));
		}
		else
			PJ_LOG(3, (THIS_FILE, "UnHold Failed, Call Doesn't have any media"));
	}
	else
		PJ_LOG(3, (THIS_FILE, "UnHold Failed, Call Not in Confirmed State"));
	return false;
}
#endif
