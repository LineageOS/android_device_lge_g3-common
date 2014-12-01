/*
 * Copyright (C) 2014 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.internal.telephony;

import static com.android.internal.telephony.RILConstants.*;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.os.Parcel;
import android.util.Log;

import com.android.internal.telephony.RILConstants;

import com.android.internal.telephony.uicc.IccCardApplicationStatus;
import com.android.internal.telephony.uicc.IccCardStatus;

/**
 * Custom Qualcomm RIL for G3
 *
 * {@hide}
 */
public class LgeLteRIL extends RIL implements CommandsInterface {
    private Message mPendingGetSimStatus;
    private Message mPendingHardwareConfig;
    private Message mPendingCdmaSubSrc;
    private int mPendingCdmaSub;

    public LgeLteRIL(Context context, int preferredNetworkType,
            int cdmaSubscription, Integer instanceId) {
        this(context, preferredNetworkType, cdmaSubscription);
    }

    public LgeLteRIL(Context context, int networkMode, int cdmaSubscription) {
        super(context, networkMode, cdmaSubscription);
    }

    @Override
    protected Object
    responseIccCardStatus(Parcel p) {
        IccCardApplicationStatus appStatus;

        IccCardStatus cardStatus = new IccCardStatus();
        cardStatus.setCardState(p.readInt());
        cardStatus.setUniversalPinState(p.readInt());
        cardStatus.mGsmUmtsSubscriptionAppIndex = p.readInt();
        cardStatus.mCdmaSubscriptionAppIndex = p.readInt();
        cardStatus.mImsSubscriptionAppIndex = p.readInt();

        int numApplications = p.readInt();

        // limit to maximum allowed applications
        if (numApplications > IccCardStatus.CARD_MAX_APPS) {
            numApplications = IccCardStatus.CARD_MAX_APPS;
        }
        cardStatus.mApplications = new IccCardApplicationStatus[numApplications];

        for (int i = 0 ; i < numApplications ; i++) {
            appStatus = new IccCardApplicationStatus();
            appStatus.app_type       = appStatus.AppTypeFromRILInt(p.readInt());
            appStatus.app_state      = appStatus.AppStateFromRILInt(p.readInt());
            appStatus.perso_substate = appStatus.PersoSubstateFromRILInt(p.readInt());
            appStatus.aid            = p.readString();
            appStatus.app_label      = p.readString();
            appStatus.pin1_replaced  = p.readInt();
            appStatus.pin1           = appStatus.PinStateFromRILInt(p.readInt());
            appStatus.pin2           = appStatus.PinStateFromRILInt(p.readInt());
            int remaining_count_pin1 = p.readInt();
            int reamining_count_puk1 = p.readInt();
            int reamining_count_pin2 = p.readInt();
            int reamining_count_puk2 = p.readInt();
            cardStatus.mApplications[i] = appStatus;
        }
        return cardStatus;
    }

    // Hack for Lollipop
    // The system now queries for SIM status before radio on, resulting
    // in getting an APPSTATE_DETECTED state. The RIL does not send an
    // RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED message after the SIM is
    // initialized, so delay the message until the radio is on.
    @Override
    public void
    getIccCardStatus(Message result) {
        if (mState != RadioState.RADIO_ON) {
            mPendingGetSimStatus = result;
        } else {
            super.getIccCardStatus(result);
        }
    }

    @Override
    public void setCdmaSubscriptionSource(int cdmaSubscription , Message response) {
        if (mState != RadioState.RADIO_ON) {
            mPendingCdmaSubSrc = response;
            mPendingCdmaSub = cdmaSubscription;
        } else {
            super.setCdmaSubscriptionSource(cdmaSubscription, response);
        }
    }

    @Override
    public void
    getHardwareConfig (Message result) {
        if (mState != RadioState.RADIO_ON) {
            mPendingHardwareConfig = result;
        } else {
            super.getHardwareConfig(result);
        }
    }

    @Override
    protected void switchToRadioState(RadioState newState) {
        super.switchToRadioState(newState);

        if (newState == RadioState.RADIO_ON) {
            if (mPendingGetSimStatus != null) {
                super.getIccCardStatus(mPendingGetSimStatus);
                mPendingGetSimStatus = null;
            }
            if (mPendingCdmaSubSrc != null) {
                super.setCdmaSubscriptionSource(mPendingCdmaSub, mPendingCdmaSubSrc);
                mPendingCdmaSubSrc = null;
            }
            if (mPendingHardwareConfig != null) {
                super.getHardwareConfig(mPendingHardwareConfig);
                mPendingHardwareConfig = null;
            }
        }
    }

    @Override
    public void setCellInfoListRate(int rateInMillis, Message response) {
        if(mRilVersion < 10) {
            if (response != null) {
                CommandException ex = new CommandException(
                    CommandException.Error.REQUEST_NOT_SUPPORTED);
                AsyncResult.forMessage(response, null, ex);
                response.sendToTarget();
            }
            return;
        }
        super.setCellInfoListRate(rateInMillis, response);
    }
}
