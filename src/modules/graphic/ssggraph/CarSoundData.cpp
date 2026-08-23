// -*- Mode: c++ -*-
/***************************************************************************
    file                 : CarSoundData.cpp
    created              : Tue Apr 5 19:57:35 CEST 2005
    copyright            : (C) 2005-2024 Christos Dimitrakakis, Bernhard Wymann
    email                : berniw@bluewin.ch

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "SoundInterface.h"
#include "CarSoundData.h"

CarSoundData::CarSoundData(int id, SoundInterface* sound_interface)
{
    eng_pri.id = id;
    eng_pri.a = 1.0f;
    engine.a = 0.0f;
    engine.f = 1.0f;
    engine.lp = 1.0f;
    smooth_accel = 0.0f;
	base_frequency = 0.0f;
    drag_collision.a = 0.0f;
    drag_collision.f = 0.0f;
    drag_collision.lp = 0.0f;	
    pre_axle = 0.0f;
    axle.a = 0.0f;
    axle.f = 0.0f;
	axle.lp = 0.0f;
    turbo.a = 0.0f;
    turbo.f = 0.0f;
	turbo.lp = 0.0f;
    engine_backfire.a=0.0f;
    engine_backfire.f=0.0f;
    engine_backfire.lp=0.0f;	
    prev_gear = 0;
    gear_changing = false;
    bottom_crash = false;
    bang = false;
    crash = false;
    turbo_on = false;
    turbo_ilag = 0.05f;
    turbo_rpm = 0.0f;
    this->sound_interface = sound_interface;
    for (int i=0; i<4; i++) {
        for (int j=0; j<3; j++) {
            wheel[i].p[j] = 0.0f;
            wheel[i].u[j] = 0.0f;
        }
        wheel[i].skid.a = 0.0f;
        wheel[i].skid.f = 1.0f;
		wheel[i].skid.lp = 0.0f;
    }
    sgVec3 zeroes = {0.0f, 0.0f, 0.0f};
    setCarPosition(zeroes);
    setCarSpeed(zeroes);
    setListenerPosition(zeroes);
	engine_sound = NULL;
    
    attenuation = 0.0f;
    grass_skid.a=0.0f;
    grass_skid.f=0.0f;
    grass_skid.lp=0.0f;
	curb.a = 0.0f;
	curb.f = 0.0f;
	curb.lp =0.0f;
    grass.a=0.0f;
    grass.f=0.0f;
    grass.lp=0.0f;
    road.a=0.0f;
    road.f=0.0f;
    road.lp=0.0f;
    skid_metal.a=0.0f;
    skid_metal.f=0.0f;
    skid_metal.lp=0.0f;
}
void CarSoundData::setEngineSound (TorcsSound* engine_sound, float rpm_scale)
{
    this->engine_sound = engine_sound;
    base_frequency = rpm_scale;
}

void CarSoundData::setTurboParameters (bool turbo_on, float turbo_rpm, float turbo_lag)
{
    this->turbo_on = turbo_on;
    this->turbo_rpm = turbo_rpm;
    if (turbo_lag > 0.0f) {
        this->turbo_ilag = exp(-3.0f*turbo_lag);
    } else {
        fprintf (stderr, "warning: turbo lag %f <= 0\n", turbo_lag);
    }
}
void CarSoundData::update (tCarElt* car)
{
    assert (car->index == eng_pri.id);
    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;
    position[0] = car->pub.DynGCg.pos.x;
    position[1] = car->pub.DynGCg.pos.y;
    position[2] = car->pub.DynGCg.pos.z;
    calculateAttenuation (car);
    calculateEngineSound (car);
    calculateBackfireSound (car);
    calculateTyreSound (car);
    calculateCollisionSound (car);
    calculateGearChangeSound (car);
}

/// Use inverse distance to calculate attenuation of sounds originating from this car. Useful for prioritisation of sounds.
void CarSoundData::calculateAttenuation (tCarElt* car)
{
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        attenuation = 0.0f;
        return;
    }
    float d = 0.0;
    for (int i=0; i<3; i++) {
        float delta = (listener_position[i] - position[i]);
        d += delta*delta;
    }
    attenuation = 1.0f / (1.0f + sqrt(d));
    eng_pri.a = attenuation;
}

/// Calculate characteristics of the engine sound.
void CarSoundData::calculateEngineSound (tCarElt* car)
{
    float mpitch = base_frequency * (float)(car->_enginerpm) / 600.0;
    engine.f = mpitch;
    engine.a = 1.0f;
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        engine.a = 0.0f;
        engine.lp = 1.0;
        turbo.a = 0.0f;
        turbo.f = 1.0f;
        return;
    }
    //assert(car->index == eng_pri.id);

    float gear_ratio = car->_gearRatio[car->_gear + car->_gearOffset];
    axle.a = 0.2f*(tanh(100.0f*(fabs(pre_axle - mpitch))));
    axle.f = (pre_axle + mpitch)*0.05f*fabs(gear_ratio);
    pre_axle = (pre_axle + mpitch)*.5;

    if (turbo_on) {
        float turbo_target = 0.1f;
        float turbo_target_vol = 0.0f;
        if (car->_enginerpm > turbo_rpm) {
            turbo_target = 0.1f + 0.9f * smooth_accel;
            turbo_target_vol = 0.1f * smooth_accel;
        }
                
        turbo.a += 0.1f * (turbo_target_vol - turbo.a) * (0.1f + smooth_accel);
        float turbo_target_pitch = turbo_target * car->_enginerpm / 600.0f;
        turbo.f += turbo_ilag * (turbo_target_pitch - turbo.f) * (smooth_accel);
        turbo.f -= turbo.f * 0.01f * (1.0-smooth_accel);//.99f;
    } else {
        turbo.a = 0.0;
    }
    smooth_accel = smooth_accel*0.5f + 0.5*(car->ctrl.accelCmd*.99f+0.01f);

    // engine filter proportional to revcor2.
    // when accel = 0, lp \in [0.0, 0.25]
    // when accel = 1, lp \in [0.25, 1.0]
    // interpolate linearly for values in between.
    float rev_cor = car->_enginerpm/car->_enginerpmRedLine;
    float rev_cor2 = rev_cor * rev_cor;
    engine.lp = (0.75f*rev_cor2 + 0.25f)*smooth_accel
        + (1.0f-smooth_accel)*0.25f*rev_cor2;

    // TODO: filter for exhaust and car body resonance?
                
}


/// Calculate the frequency and amplitude of a looped backfiring sound.
void CarSoundData::calculateBackfireSound (tCarElt* car)
{               
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        engine_backfire.a = 0.0f;
        engine_backfire.f = 1.0f;
        return;
    }
    if ((car->priv.smoke>0.0)&&(engine_backfire.a<0.5)) {
        engine_backfire.a += .25f*car->priv.smoke;
    }
    engine_backfire.f = ((float)(car->_enginerpm) / 600.0);
    engine_backfire.a *= (.9*.5+.5*exp(-engine_backfire.f));
}

void CarSoundData::calculateTyreSound(tCarElt* car)
{
	// Initialize sound parameters
	grass_skid.a = grass.a = 0.0f;
	grass.f = curb.f = 1.0f;
	curb.a = 0.0f;
	road.a = road.f = 0.0f;
	
	// Initialize wheel sounds
	int wheelIndex;
	for (wheelIndex = 0; wheelIndex < 4; wheelIndex++) {
		wheel[wheelIndex].skid.a = 0.0f;
		wheel[wheelIndex].skid.f = 1.0f;
	}

	// Early exit if the car simulation is not active
	if (car->_state & RM_CAR_STATE_NO_SIMU) {
		return;
	}

	// Check if any wheel is spinning significantly
	bool wheelSpinning = false;
	for (wheelIndex = 0; wheelIndex < 4; wheelIndex++) {
		if (car->_wheelSpinVel(wheelIndex) > 0.1f) {
			wheelSpinning = true;
			break;
		}
	}

	// Early exit if the car is stationary and wheels are not spinning
	if ((car->pub.speed < 0.3f) && !wheelSpinning) {
		return;
	}

    for (wheelIndex = 0; wheelIndex<4; wheelIndex++) {
		// I do not skip here for car->_reaction[i] <= 0, because the road sample currently also includes airflow noise
        tdble tmpvol = car->pub.speed*0.01f;
        tdble roughnessFreq = calculateRoughnessFreqency(car->priv.wheel[wheelIndex].seg);
        tdble ride = 0.001f * car->_reaction[wheelIndex];

		// Get information about tire overlapping another surface (e.g. at road edge to the curb).
		tdble otherSurfaceContribution = 0.0f;
		tdble otherRoughnessFreq = 1.0f;
		bool onOtherSurface = car->priv.otherSurfaceContribution[wheelIndex] > 0.0f && car->priv.otherSurfaceSeg[wheelIndex] != NULL;

		if (onOtherSurface) {
			otherSurfaceContribution = car->priv.otherSurfaceContribution[wheelIndex];
			otherRoughnessFreq = calculateRoughnessFreqency(car->priv.otherSurfaceSeg[wheelIndex]);
		}

		// Curb handling, implicit assumption that there are never two curbs directly side by side. This
		// is currently guaranteed.

		// Check curb sound contribution, skip calculation if there is no load on the wheel (no sound)
		// The Curb effect is an "addon" effect, it does not exclude the road or dirt effects, e.g. you
		// can have the tires skid on the curbs and have wind noise.
		handleCurbContribution(&car->priv, onOtherSurface, otherSurfaceContribution, roughnessFreq,
			otherRoughnessFreq, tmpvol, ride, wheelIndex, car->_reaction[wheelIndex]);
		
		// Handling of dirt and normal surfaces, here we can have an overlapping tire spanning either
		// two surfaces of the same type or a different type.
		bool mainSurfaceIsOffroad = isOffRoadSurface(car->priv.wheel[wheelIndex].seg);
		bool onOtherDifferentSurface =
			onOtherSurface && (mainSurfaceIsOffroad != isOffRoadSurface(car->priv.otherSurfaceSeg[wheelIndex]));

		tdble roadContribution = 0.0f;
		tdble dirtContribution = 0.0f;

		if (onOtherDifferentSurface) {
			// Mixed cases, two different surfaces
			if (mainSurfaceIsOffroad) {
				dirtContribution = 1.0f - otherSurfaceContribution;
				roadContribution = otherSurfaceContribution;
			} else {
				roadContribution = 1.0f - otherSurfaceContribution;
				dirtContribution = otherSurfaceContribution;
			}
		} else {
			// Both surfaces of the same type
			if (mainSurfaceIsOffroad) {
				dirtContribution = 1.0f;
			} else {
				roadContribution = 1.0f;
			}
		}

		// Normal road handling
		if (roadContribution > 0.0f) {
			handleRoadContribution(mainSurfaceIsOffroad, roadContribution, roughnessFreq,
				otherRoughnessFreq, tmpvol, ride, wheelIndex, car->_skid[wheelIndex],
				car->_wheelSlipAccel(wheelIndex), car->_reaction[wheelIndex]);
		}

		// Dirt handling
		if (dirtContribution > 0.0f) {
			tdble dirtRoughnessFreq = 0.0f;
			tdble dirtRoughness = 0.0f;

			getDirtRoughnessParams(car, wheelIndex, mainSurfaceIsOffroad, roughnessFreq, otherRoughnessFreq, dirtRoughnessFreq, dirtRoughness);
			handleDirtContribution(dirtContribution, dirtRoughnessFreq, dirtRoughness, car->_skid[wheelIndex], tmpvol, ride);
		}
	}

    for (wheelIndex = 0; wheelIndex<4; wheelIndex++) {
        tdble az = car->_yaw;
        tdble Sinz = sin(az);
        tdble Cosz = cos(az);
                
        tdble x = car->priv.wheel[wheelIndex].relPos.x;
        tdble y = car->priv.wheel[wheelIndex].relPos.y;
                
        tdble dx = x * Cosz - y * Sinz;
        tdble dy = x * Sinz + y * Cosz;
                
        tdble dux_initial = -car->_yaw_rate * y;
        tdble duy_initial = car->_yaw_rate * x;

		tdble dux = dux_initial * Cosz - duy_initial * Sinz;
		tdble duy = dux_initial * Sinz + duy_initial * Cosz;
                
        wheel[wheelIndex].u[0] = car->pub.DynGCg.vel.x + dux;
        wheel[wheelIndex].u[1] = car->pub.DynGCg.vel.y + duy;
        wheel[wheelIndex].u[2] = car->pub.DynGCg.vel.z;
        wheel[wheelIndex].p[0] = car->pub.DynGCg.pos.x + dx;
        wheel[wheelIndex].p[1] = car->pub.DynGCg.pos.y + dy;
        wheel[wheelIndex].p[2] = car->pub.DynGCg.pos.z;
    }
}

bool CarSoundData::isOffRoadSurface(const tTrackSeg* const seg) {
	const char* const materialName = seg->surface->material;
	return materialName &&
		(strstr(materialName, TRK_VAL_SAND)
		|| strstr(materialName, TRK_VAL_DIRT)
		|| strstr(materialName, TRK_VAL_GRASS)
		|| strstr(materialName, "gravel")
		|| strstr(materialName, "mud")
        );
}


tdble CarSoundData::calculateRoughnessFreqency(const tTrackSeg* const seg)
{
	tdble roughnessFreq = 2.0f*EX_PI*seg->surface->kRoughWaveLen;
	if (roughnessFreq>2.0f) {
		roughnessFreq = 2.0f + tanh(roughnessFreq-2.0f);
	}
	return roughnessFreq;
}


void CarSoundData::getDirtRoughnessParams(
	tCarElt* car,
	int wheelIndex,
	bool mainSurfaceIsOffroad,
	tdble roughnessFreq,
	tdble otherRoughnessFreq,
	tdble& dirtRoughnessFreq,
	tdble& dirtRoughness
) {
	if (mainSurfaceIsOffroad) {
		dirtRoughnessFreq = roughnessFreq;
		dirtRoughness = car->priv.wheel[wheelIndex].seg->surface->kRoughness;
	} else {
		dirtRoughnessFreq = otherRoughnessFreq;
		dirtRoughness = car->priv.otherSurfaceSeg[wheelIndex]->surface->kRoughness;
	}
}


void CarSoundData::handleDirtContribution(
	tdble dirtContribution,
	tdble dirtRoughnessFreq,
	tdble dirtRoughness,
	tdble wheelSkid,
	tdble tmpvol,
	tdble ride
) {
	if (dirtContribution > 0.0f) {
		tdble dirtpitch = tmpvol * (0.5f + 0.5f*dirtRoughnessFreq);
		tdble dirtvol = (0.5f + 0.2f*tanh(0.5f*dirtRoughness))*tmpvol*ride*dirtContribution;

		if (grass.a < dirtvol) {
			grass.a = dirtvol;
			grass.f = dirtpitch;
		}

		tdble grassvol = wheelSkid*dirtContribution;
		if (grass_skid.a < grassvol) {
			grass_skid.a = static_cast<float>(grassvol);
			grass_skid.f = 1.0f;
		}
	}
}


void CarSoundData::handleRoadContribution(
	bool mainSurfaceIsOffroad,
	tdble roadContribution,
	tdble roughnessFreq,
	tdble otherRoughnessFreq,
	tdble tmpvol,
	tdble ride,
	int wheelIndex,
	tdble wheelSkid,
	tdble wheelSlipAccel,
	tdble wheelReaction
) {
	if (roadContribution > 0.0f) {
		tdble roadRoughnessFreq;
		if (!mainSurfaceIsOffroad) {
			roadRoughnessFreq = roughnessFreq;
		} else {
			roadRoughnessFreq = otherRoughnessFreq;
		}

		tdble roadpitch = tmpvol*(0.75f+0.25f*roadRoughnessFreq);
		tdble wind_noise = 1.0f;
		tdble road_noise = 0.25f;
		tdble roadvol = tmpvol*(wind_noise + ride*road_noise)*roadContribution;
		if (road.a < roadvol) {
			road.a = roadvol;
			road.f = roadpitch;
		}

		if (wheelSkid > 0.05f) {
			wheel[wheelIndex].skid.a = (float)(wheelSkid-0.05f)*roadContribution;
			float wsa = tanh((wheelSlipAccel+10.0f)*0.01f);
			wheel[wheelIndex].skid.f = (0.3f - 0.3f*wsa + 0.3f*roadRoughnessFreq)/(1.0f+0.5f*tanh(wheelReaction*0.0001f));
		}
	}
}


void CarSoundData::handleCurbContribution(
	tPrivCar* car,
	bool onOtherSurface,
	tdble otherSurfaceContribution,
	tdble curbRoughnessFreq,
	tdble otherRoughnessFreq,
	tdble tmpvol,
	tdble ride,
	int wheelIndex,
	tdble wheelReaction
) {
	if (wheelReaction > 0.0f) {
		tdble curbContribution = 0.0f;
		if (car->wheel[wheelIndex].seg->style == TR_CURB) {
			curbContribution = 1.0f - otherSurfaceContribution;
		} else if (onOtherSurface && car->otherSurfaceSeg[wheelIndex]->style == TR_CURB) {
			curbContribution = otherSurfaceContribution;
			curbRoughnessFreq = otherRoughnessFreq;
		}

		if (curbContribution > 0.0f) {
			// Constants by trial and error, there is no deeper reasoning behind it
			float curbpitch = tmpvol*(0.75f+0.25f*curbRoughnessFreq);
			float curbvol = tmpvol*(5.0f + ride/3.0f)*curbContribution;
			// There is only one effect for the car, take the loudest wheel
			if (curb.a < curbvol) {
				curb.a = curbvol;
				curb.f = curbpitch;
			}
		}
	}
}


void CarSoundData::calculateGearChangeSound (tCarElt* car) {
    if (car->_gear != prev_gear) {
        prev_gear = car->_gear;
        gear_changing = true;
    } else {
        gear_changing = false;
    }

}

void CarSoundData::calculateCollisionSound (tCarElt* car)
{
    skid_metal.a = 0.0f;
    skid_metal.f = 1.0f;
    bottom_crash = false;
    bang = false;
    crash = false;
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        return;
    }
                

        
    int collision  = car->priv.collision;
    if (collision) {
        if (collision & 1) {
            skid_metal.a = car->pub.speed*0.01;
            skid_metal.f = .5+0.5*skid_metal.a;
            drag_collision.f = skid_metal.f;
        } else {
            skid_metal.a = 0;
        }

        if ((collision & 16 )) {
            bottom_crash = true;
        }

        if ((collision & 8 )) {
            bang = true;
        }

        if (((collision & 1) ==0) ||
            ((collision & 2)
             &&(skid_metal.a >drag_collision.a))) {
            crash = true;
        }
        car->priv.collision = 0;
    }

    drag_collision.a = 0.9f*drag_collision.a + skid_metal.a;
    if (drag_collision.a>1.0f) {
        drag_collision.a = 1.0f;
    }
    skid_metal.a = drag_collision.a;
    skid_metal.f = drag_collision.f;
}
