// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

//Student Soccer 2D Simulation Base , STDAGENT2D
//Simplified the Agent2D Base for HighSchool Students.
//Technical Committee of Soccer 2D Simulation League, IranOpen
//Nader Zare
//Mostafa Sayahi
//Pooria Kaviani
/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bhv_basic_offensive_kick.h"
#include "bhv_basic_ai.h"

#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_smart_kick.h>
#include <rcsc/action/body_stop_ball.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include <rcsc/geom/sector_2d.h>

#include <vector>

#include "classes/ai_agent.h"
#include "bhv_basic_ai.h"
using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
bool
Bhv_BasicOffensiveKick::execute( PlayerAgent * agent )
{
    int a;
    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicOffensiveKick" );

    const WorldModel & wm = agent->world();

    
    if(perfectShoot(agent)){
    	return true;
    }
    
    if(shoot(agent)){
    	return true;
    }

    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< PlayerObject * >( 0 )
            : opps.front() );
    const double nearest_opp_dist = ( nearest_opp
                                      ? nearest_opp->distFromSelf()
                                      : 1000.0 );
//    const Vector2D nearest_opp_pos = ( nearest_opp
//                                       ? nearest_opp->pos()
//                                       : Vector2D( -1000.0, 0.0 ) );

    if(nearest_opp_dist < 10){
    	if(pass(agent))
    		return true;
    }



    if(dribble(agent)){
    	return true;
    }

    if ( nearest_opp_dist > 2.5 )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": hold" );
        agent->debugClient().addMessage( "OffKickHold" );
        Body_HoldBall().execute( agent );
        return true;
    }
    clearball(agent);
    return true;
}

bool Bhv_BasicOffensiveKick::shoot( rcsc::PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();
    Vector2D ball_pos = wm.ball().pos();
    Vector2D leftpos[7],rightpos[7];
    double y=-6;
    if(ball_pos.x<35)
    {
        return false;
    }
    for(int i=1;i<=6;++i)
    {
        leftpos[i].assign(52.5,y);
        ++y;
    }
    y=6;
    for(int i=1;i<=6;++i)
    {
        rightpos[i].assign(52.5,y);
        --y;
    }
    if(ball_pos.y<0)
    {
        for(int i=1;i<=6;++i)
        {
            for(double s=2.7;s>2.2;s=s-0.1)
            {
                if(safepathshoot(agent,leftpos[i],s))
                {
                    return Body_SmartKick(leftpos[i],s,s,3).execute(agent); 
                }        
            }          
        }
        for(int i=1;i<=6;++i)
        {
            for(double s=2.7;s>2.2;s=s-0.1)
            {
                if(safepathshoot(agent,rightpos[i],s))
                {
                    return Body_SmartKick(rightpos[i],s,s,3).execute(agent); 
                }        
            }          
        }
    }
    else
    {
        for(int i=1;i<=6;++i)
        {
            for(double s=2.7;s>2.2;s=s-0.1)
            {
                if(safepath(agent,rightpos[i],s))
                {
                    return Body_SmartKick(rightpos[i],s,s,3).execute(agent); 
                }        
            }          
        }
        for(int i=1;i<=6;++i)
        {
            for(double s=2.7;s>2.2;s=s-0.1)
            {
                if(safepath(agent,leftpos[i],s))
                {
                    return Body_SmartKick(leftpos[i],s,s,3).execute(agent); 
                }        
            }          
        }
    }
    return false;

}
bool Bhv_BasicOffensiveKick::safepath(rcsc::PlayerAgent* agent, rcsc::Vector2D target, double s)
{
    const WorldModel &wm=agent->world();
    Vector2D ball_pos=wm.ball().pos();
    AngleDeg an=(target-ball_pos).th();
    Vector2D pos_vel=Vector2D::polar2vector(s,an),n_pos;
    double distop,disttm;
    for(int i=1;i<=ball_step2(agent,target,s);++i)
    {
        n_pos=inertia_n_step_point(ball_pos,pos_vel,i,ServerParam::i().ballDecay());
        
        distop=wm.getDistOpponentNearestTo(n_pos,5);
        disttm=wm.getDistTeammateNearestTo(n_pos,5);
        if(distop<i+1)
        {
            return false;
        }
    }
    return true;
}

int Bhv_BasicOffensiveKick::ball_step2(PlayerAgent* agent,Vector2D tm_pos,double sp)
{
    const WorldModel &wm=agent->world();
    Vector2D ball_pos=wm.ball().pos();
    Vector2D v1,v2,pos_vel;
    AngleDeg t=(tm_pos-ball_pos).th();
    bool w=true;
    double dist1,dist2;
    int i=1;
    pos_vel=Vector2D::polar2vector(sp,t);
    v1=inertia_n_step_point(ball_pos,pos_vel,i,ServerParam::i().ballDecay());
    v2=inertia_n_step_point(ball_pos,pos_vel,i+1,ServerParam::i().ballDecay());
    dist1=v1.dist(tm_pos);
    dist2=v2.dist(tm_pos);
    while(dist2<dist1 && i<20)
    {
        if(dist2<0.3)
        {
                return i+1;
        }
            ++i;
            v1=inertia_n_step_point(ball_pos,pos_vel,i,ServerParam::i().ballDecay());
            v2=inertia_n_step_point(ball_pos,pos_vel,i+1,ServerParam::i().ballDecay());
            dist1=v1.dist(tm_pos);
            dist2=v2.dist(tm_pos);
    }
    return i;
}

bool Bhv_BasicOffensiveKick::safepathshoot(rcsc::PlayerAgent* agent, rcsc::Vector2D target, double s)
{
    const WorldModel &wm=agent->world();
    Vector2D ball_pos=wm.ball().pos();
    AngleDeg an=(target-ball_pos).th();
    Vector2D pos_vel=Vector2D::polar2vector(s,an),n_pos;
    double distop,disttm;
    int step=ball_step2(agent,target,s);
    for(int i=1;i<=step;++i)
    {
        n_pos=inertia_n_step_point(ball_pos,pos_vel,i,ServerParam::i().ballDecay());
        
        distop=wm.getDistOpponentNearestTo(n_pos,5);
        disttm=wm.getDistTeammateNearestTo(n_pos,5);
        if(distop<i)
        {
            return false;
        }
    }
    //agent->debugClient().addCircle(n_pos,3);
    return true;
}


bool Bhv_BasicOffensiveKick::pass(PlayerAgent * agent){
	const WorldModel & wm = agent->world();
	std::vector<Vector2D> targets;
	Vector2D ball_pos = wm.ball().pos();
	for(int u = 1;u<=11;u++){
		const AbstractPlayerObject * tm = wm.ourPlayer(u);
		if(tm==NULL || tm->unum() < 1 || tm->unum() == wm.self().unum() )
			continue;
		Vector2D tm_pos = tm->pos();
		if(tm->pos().dist(ball_pos) > 30)
			continue;
		Sector2D pass = Sector2D(ball_pos,1,tm_pos.dist(ball_pos)+3,(tm_pos - ball_pos).th() - 15,(tm_pos - ball_pos).th() + 15);
		if(!wm.existOpponentIn(pass,5,true)){
			targets.push_back(tm_pos);
		}
	}
	if(targets.size() == 0)
		return false;
	Vector2D best_target = targets[0];
    for(unsigned int i=1;i<targets.size();i++){
		if(targets[i].x > best_target.x)
			best_target = targets[i];
	}
	if(wm.gameMode().type()!= GameMode::PlayOn)
        Body_SmartKick(best_target,3,2.5,1).execute(agent);
	else
        Body_SmartKick(best_target,3,2.5,2).execute(agent);
	return true;
}

bool Bhv_BasicOffensiveKick::dribble(PlayerAgent * agent)
{
// 	const WorldModel & wm = agent->world();
// 	Vector2D ball_pos = wm.ball().pos();
// 	double dribble_angle = (Vector2D(52.5,0) - ball_pos).th().degree();
// 	Sector2D dribble_sector = Sector2D(ball_pos,0,3,dribble_angle - 15,dribble_angle+15);
// 	if(!wm.existOpponentIn(dribble_sector,5,true)){
// 		Vector2D target = Vector2D::polar2vector(3,dribble_angle) + ball_pos;
// 		if(Body_SmartKick(target,0.8,0.7,2).execute(agent)){
// 			return true;
// 		}
// 	}
// 	return false;
    double direction = bhv_basic_ai().penalty_execute(agent);
    return agent->doKick(direction , 0.2 );
}

bool Bhv_BasicOffensiveKick::clearball(PlayerAgent * agent){
    const WorldModel & wm = agent->world();
    if(!wm.self().isKickable())
        return false;
    Vector2D ball_pos = wm.ball().pos();
    Vector2D target = Vector2D(52.5,0);
    if(ball_pos.x < 0){
        if(ball_pos.x > -25){
            if(ball_pos.dist(Vector2D(0,-34)) < ball_pos.dist(Vector2D(0,+34))){
                target = Vector2D(0,-34);
            }else{
                target = Vector2D(0,+34);
            }
        }else{
            if(ball_pos.absY() < 10 && ball_pos.x < -10){
                if(ball_pos.y > 0){
                    target = Vector2D(-52,20);
                }else{
                    target = Vector2D(-52,-20);
                }
            }else{
                if(ball_pos.y > 0){
                    target = Vector2D(ball_pos.x,34);
                }else{
                    target = Vector2D(ball_pos.x,-34);
                }
            }
        }
    }
    if(Body_SmartKick(target,2.7,2.7,2).execute(agent)){
        return true;
    }
    Body_StopBall().execute(agent);
    return true;
}

