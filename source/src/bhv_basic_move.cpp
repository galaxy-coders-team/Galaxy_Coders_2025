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

#include "bhv_basic_move.h"

#include "bhv_basic_tackle.h"
#include "bhv_basic_offensive_kick.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/say_message_builder.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include <vector>
#include <cstdio>

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
bool
Bhv_BasicMove::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove" );

    //-----------------------------------------------
    // tackle

    const WorldModel & wm = agent->world();
    agent->debugClient().addMessage("size %3f ",wm.self().kickRate());
    const PlayerObject *tm=wm.getTeammateNearestToBall(5);
    agent->doAttentionto(wm.ourSide(),tm->unum());
    /*--------------------------------------------------------*/
    // chase ball
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    agent->debugClient().addMessage("self = %d tm = %d op = %d",self_min,mate_min,opp_min);
    const Vector2D vec_intercept =wm.ball().inertiaPoint(self_min);
    Vector2D target_point;
    if ( Bhv_BasicTackle( 0.90, 80.0 ).execute( agent )&& wm.self().distFromBall()>wm.getDistOpponentNearestToBall(5)  )
    {
        return true;
    }
    if(opp_min<self_min && opp_min<mate_min)
    {
        target_point = def_position( agent,wm.self().unum() );
    }
    else
    {
        target_point = at_position(agent,wm.self().unum());
    }
    if(wm.gameMode().isOurSetPlay(wm.ourSide()) && wm.gameMode().type()==GameMode::FoulCharge_)
    {
        target_point = def_position( agent,wm.self().unum() );
    }
/*    if( self_min<=mate_min )
    {
        if(!Body_GoToPoint(vec_intercept,0.1,100).execute(agent))
        {agent->debugClient().addMessage("turn_self");
            Body_TurnToBall().execute(agent);
            return true;
        }
        agent->debugClient().addMessage("intercept_self");
        return true;
        
    }*/
    if(!wm.existKickableTeammate() && ((self_min<=mate_min) || wm.self().distFromBall()<2 ))
    {
        Body_Intercept().execute(agent);
        agent->debugClient().addCircle(wm.ball().inertiaPoint(self_min),0.2);
        agent->debugClient().addMessage("intercept");
        return true;  
    }    
    const double dash_power = get_normal_dash_power( wm );
    if(mark(agent) && opp_min<self_min && opp_min<mate_min)
    {
        return true;
    }
    if ( ! Body_GoToPoint( target_point, 2, dash_power
                           ).execute( agent ) )
    {
        Body_TurnToBall().execute( agent );
    }
    return true;
}

rcsc::Vector2D Bhv_BasicMove::getPosition(const rcsc::WorldModel & wm, int self_unum){
    int ball_step = 0;
    if ( wm.gameMode().type() == GameMode::PlayOn
         || wm.gameMode().type() == GameMode::GoalKick_ )
    {
        ball_step = std::min( 1000, wm.interceptTable()->teammateReachCycle() );
        ball_step = std::min( ball_step, wm.interceptTable()->opponentReachCycle() );
        ball_step = std::min( ball_step, wm.interceptTable()->selfReachCycle() );
    }

    Vector2D ball_pos = wm.ball().inertiaPoint( ball_step );

   /* dlog.addText( Logger::TEAM,
                  __FILE__": HOME POSITION: ball pos=(%.1f %.1f) step=%d",
                  ball_pos.x, ball_pos.y,
                  ball_step );*/

    std::vector<Vector2D> positions(12);
    double min_x_rectengle[12]={0,-52,-52,-52,-52,-52,-30,-30,-30,0,0,0};
    double max_x_rectengle[12]={0,-48,-10,-10,-10,-10,15,15,15,50,50,50};
    double min_y_rectengle[12]={0,-2,-20,-10,-30,10,-20,-30, 0,-20,-30, 0};
    double max_y_rectengle[12]={0,+2, 10, 20,-10,30, 20, 0,30, 20, 0, 30};

    for(int i=1; i<=11; i++){
          double xx_rectengle = max_x_rectengle[i] - min_x_rectengle[i];
          double yy_rectengle = max_y_rectengle[i] - min_y_rectengle[i];
          double x_ball = ball_pos.x + 52.5;
          x_ball /= 105.5;
          double y_ball = ball_pos.y + 34;
          y_ball /= 68.0;
          double x_pos = xx_rectengle * x_ball + min_x_rectengle[i];
          double y_pos = yy_rectengle * y_ball + min_y_rectengle[i];
          positions[i] = Vector2D(x_pos,y_pos);
    }

    if ( ServerParam::i().useOffside() )
    {
        double max_x = wm.offsideLineX();
        if ( ServerParam::i().kickoffOffside()
             && ( wm.gameMode().type() == GameMode::BeforeKickOff
                  || wm.gameMode().type() == GameMode::AfterGoal_ ) )
        {
            max_x = 0.0;
        }
        else
        {
            int mate_step = wm.interceptTable()->teammateReachCycle();
            if ( mate_step < 50 )
            {
                Vector2D trap_pos = wm.ball().inertiaPoint( mate_step );
                if ( trap_pos.x > max_x ) max_x = trap_pos.x;
            }

            max_x -= 1.0;
        }

        for ( int unum = 1; unum <= 11; ++unum )
        {
            if ( positions[unum].x > max_x )
            {
              /*  dlog.addText( Logger::TEAM,
                              "____ %d offside. home_pos_x %.2f -> %.2f",
                              unum,
                              positions[unum].x, max_x );
                positions[unum].x = max_x;*/
            }
        }
    }
    return positions.at(self_unum);
}

double Bhv_BasicMove::get_normal_dash_power( const WorldModel & wm )
{
    static bool s_recover_mode = false;

    if ( wm.self().staminaModel().capacityIsEmpty() )
    {
        return std::min( ServerParam::i().maxDashPower(),
                         wm.self().stamina() + wm.self().playerType().extraStamina() );
    }

    // check recover
    if ( wm.self().staminaModel().capacityIsEmpty() )
    {
        s_recover_mode = false;
    }
    else if ( wm.self().stamina() < ServerParam::i().staminaMax() * 0.5 )
    {
        s_recover_mode = true;
    }
    else if ( wm.self().stamina() > ServerParam::i().staminaMax() * 0.7 )
    {
        s_recover_mode = false;
    }

    /*--------------------------------------------------------*/
    double dash_power = ServerParam::i().maxDashPower();
    const double my_inc
        = wm.self().playerType().staminaIncMax()
        * wm.self().recovery();

    if ( wm.ourDefenseLineX() > wm.self().pos().x
         && wm.ball().pos().x < wm.ourDefenseLineX() + 20.0 )
    {
    }
    else if ( s_recover_mode )
    {
    }
    else if ( wm.existKickableTeammate()
              && wm.ball().distFromSelf() < 20.0 )
    {
    }
    else if ( wm.self().pos().x > wm.offsideLineX() )
    {
    }
    else
    {
        dash_power = std::min( my_inc * 1.7,
                               ServerParam::i().maxDashPower() );
    }

    return dash_power;
}

Vector2D Bhv_BasicMove::def_position(rcsc::PlayerAgent* agent,int u)
{
    const WorldModel &wm=agent->world();
    int point=areanumber_def(wm);
    Vector2D ball_area=areapos_def(wm);
    Vector2D ball_pos=wm.ball().pos();
    
    Vector2D pos[12];
    Vector2D pos2[10];
    pos[0]=areapos_def(wm);
    Vector2D ball_pos1;
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(self_min<opp_min && self_min < mate_min )
    {
        ball_pos1=wm.ball().inertiaPoint(self_min);
    }
    else if(mate_min<opp_min && mate_min<opp_min )
    {
        ball_pos1=wm.ball().inertiaPoint(mate_min);
    }
    else if(opp_min<self_min && opp_min<mate_min )
    {
        ball_pos1=wm.ball().inertiaPoint(opp_min);
    }
    pos2[0]=ball_pos1;
    Vector2D e=ball_pos1-ball_area;
    Vector2D team[12];
    std::vector <const AbstractPlayerObject *> teammate1;
    std::vector <Vector2D> team1;
    const AbstractPlayerObject *teammate[12];
    const AbstractPlayerObject *teammates[10][12];
    const AbstractPlayerObject *tm_end[10];
	for(int u = 2;u<=11;u++)
    {
		const AbstractPlayerObject * tm = wm.ourPlayer(u);
		if(tm==NULL || tm->unum() < 1)
        {
            continue;
        }
		team[u]=tm->pos();
        teammate[u]=tm;
        team1.push_back(tm->pos());
        teammate1.push_back(tm);
	}

    if(point == 1)
    {
    pos[2].assign(-49.35,-20.58);
    pos[3].assign(-45.21,-12.97);
    pos[4].assign(-45.72,-4.35);
    pos[5].assign(-41.83,-19.31);
    pos[6].assign(-36.34,-10.69);
    pos[7].assign(-46.31,4.1);
    pos[8].assign(-37.01,-0.89);
    pos[9].assign(-33.55,-16.94);
    pos[10].assign(-40.23,-27.17);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 2)
    {
    pos[2].assign(-46.82,-20.58);
    pos[3].assign(-45.21,-12.97);
    pos[4].assign(-45.72,-4.35);
    pos[5].assign(-40.56,-17.62);
    pos[6].assign(-36.34,-10.69);
    pos[7].assign(-46.31,4.1);
    pos[8].assign(-37.01,-0.89);
    pos[9].assign(-33.55,-16.94);
    pos[10].assign(-39.72,-27.17);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 3)
    {
    pos[2].assign(-46.82,-20.58);
    pos[3].assign(-45.21,-12.97);
    pos[4].assign(-45.72,-4.35);
    pos[5].assign(-40.56,-17.62);
    pos[6].assign(-36.34,-10.69);
    pos[7].assign(-46.31,4.1);
    pos[8].assign(-37.01,-0.89);
    pos[9].assign(-33.55,-16.94);
    pos[10].assign(-39.72,-27.17);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 4)
    {
    pos[2].assign(-44.45,-21.25);
    pos[3].assign(-43.77,-13.39);
    pos[4].assign(-44.03,-4.86);
    pos[5].assign(-38.37,-18.46);
    pos[6].assign(-36.34,-10.69);
    pos[7].assign(-45.04,4.1);
    pos[8].assign(-37.01,-0.89);
    pos[9].assign(-30.51,-17.54);
    pos[10].assign(-34.48,-26.75);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 5)
    {
    pos[2].assign(-41.07,-21.08);
    pos[3].assign(-41.15,-12.8);
    pos[4].assign(-41.58,-4.35);
    pos[5].assign(-34.39,-18.72);
    pos[6].assign(-32.54,-10.69);
    pos[7].assign(-41.92,4.69);
    pos[8].assign(-32.87,0.13);
    pos[9].assign(-26.11,-17.37);
    pos[10].assign(-30.25,-23.7);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 6)
    {
    pos[2].assign(-38.45,-21.08);
    pos[3].assign(-39.04,-13.14);
    pos[4].assign(-39.04,-4.18);
    pos[5].assign(-30.42,-18.97);
    pos[6].assign(-28.39,-10.27);
    pos[7].assign(-38.79,4.94);
    pos[8].assign(-28.9,-0.97);
    pos[9].assign(-21.13,-16.94);
    pos[10].assign(-25.86,-23.37);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 7)
    {
    pos[2].assign(-34.23,-21.42);
    pos[3].assign(-34.56,-13.39);
    pos[4].assign(-34.99,-3.85);
    pos[5].assign(-26.62,-18.3);
    pos[6].assign(-22.14,-10.69);
    pos[7].assign(-34.31,4.01);
    pos[8].assign(-24.76,-1.23);
    pos[9].assign(-14.7,-16.44);
    pos[10].assign(-20.2,-24.04);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 8)
    {
    pos[2].assign(-32.7,-21.59);
    pos[3].assign(-32.96,-13.31);
    pos[4].assign(-33.04,-3.68);
    pos[5].assign(-24.17,-18.72);
    pos[6].assign(-20.11,-10.86);
    pos[7].assign(-32.28,3.59);
    pos[8].assign(-24.76,-1.23);
    pos[9].assign(-12.68,-16.01);
    pos[10].assign(-18,-23.79);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 9)
    {
    pos[2].assign(-29.75,-21.68);
    pos[3].assign(-30.42,-13.39);
    pos[4].assign(-30.42,-3.59);
    pos[5].assign(-21.55,-18.89);
    pos[6].assign(-16.82,-11.28);
    pos[7].assign(-29.66,4.94);
    pos[8].assign(-22.39,-4.69);
    pos[9].assign(-8.11,-16.61);
    pos[10].assign(-14.2,-23.79);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 10)
    {
    pos[2].assign(-27.13,-22.35);
    pos[3].assign(-28.14,-13.56);
    pos[4].assign(-28.31,-3.51);
    pos[5].assign(-18.08,-18.8);
    pos[6].assign(-13.44,-11.11);
    pos[7].assign(-27.46,4.86);
    pos[8].assign(-19.61,-4.27);
    pos[9].assign(-2.96,-16.18);
    pos[10].assign(-10.14,-23.62);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 11)
    {
    pos[2].assign(-24.08,-22.18);
    pos[3].assign(-24.34,-13.14);
    pos[4].assign(-24.93,-3.85);
    pos[5].assign(-24.68,4.44);
    pos[6].assign(-9.89,-17.54);
    pos[7].assign(-13.44,-4.01);
    pos[8].assign(-3.55,-7.82);
    pos[9].assign(4.9,-16.35);
    pos[10].assign(-1.18,-24.63);
    pos[11].assign(7.41,28.44);
    }
    else if(point == 12)
    {
    pos[2].assign(-18.17,-22.01);
    pos[3].assign(-18.68,-13.14);
    pos[4].assign(-18.42,-4.5);
    pos[5].assign(-17.83,5.45);
    pos[6].assign(-2.7,-17.79);
    pos[7].assign(-6,-3.25);
    pos[8].assign(7.01,-8.92);
    pos[9].assign(18.85,-16.1);
    pos[10].assign(7.18,-24.8);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 13)
    {
    pos[2].assign(-12.17,-21.42);
    pos[3].assign(-12.34,-12.89);
    pos[4].assign(-12.08,-4.35);
    pos[5].assign(-12.17,5.2);
    pos[6].assign(3.21,-17.62);
    pos[7].assign(3.04,-1.9);
    pos[8].assign(15.55,-8.75);
    pos[9].assign(24.85,-15.42);
    pos[10].assign(15.63,-22.86);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 14)
    {
    pos[2].assign(-5.24,-21.42);
    pos[3].assign(-5.66,-12.72);
    pos[4].assign(-6.34,-3.76);
    pos[5].assign(-6.42,5.37);
    pos[6].assign(10.73,-17.7);
    pos[7].assign(10.9,-1.65);
    pos[8].assign(24.17,-10.61);
    pos[9].assign(34.9,-15.25);
    pos[10].assign(23.92,-22.94);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 15)
    {
    pos[2].assign(-0.59,-21.08);
    pos[3].assign(-0.59,-12.04);
    pos[4].assign(-1.01,-3);
    pos[5].assign(-0.25,6.46);
    pos[6].assign(17.24,-18.38);
    pos[7].assign(16.23,-2.49);
    pos[8].assign(28.48,-10.01);
    pos[9].assign(40.73,-15.17);
    pos[10].assign(30.59,-23.03);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 16)
    {
    pos[2].assign(-49.77,-17.11);
    pos[3].assign(-46.23,-8.66);
    pos[4].assign(-45.38,-0.21);
    pos[5].assign(-41.83,6.13);
    pos[6].assign(-40.06,-10.27);
    pos[7].assign(-42.42,-17.37);
    pos[8].assign(-39.21,-2.58);
    pos[9].assign(-34.48,-20.92);
    pos[10].assign(-33.46,-13.23);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 17)
    {
    pos[2].assign(-46.56,-17.03);
    pos[3].assign(-46.23,-8.66);
    pos[4].assign(-45.38,-0.21);
    pos[5].assign(-41.83,6.13);
    pos[6].assign(-40.06,-10.27);
    pos[7].assign(-38.62,-17.45);
    pos[8].assign(-39.21,-2.58);
    pos[9].assign(-32.96,-21.17);
    pos[10].assign(-31.69,-11.45);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 18)
    {
    pos[2].assign(-44.62,-16.52);
    pos[3].assign(-45.21,-8.92);
    pos[4].assign(-44.54,-0.63);
    pos[5].assign(-41.83,6.13);
    pos[6].assign(-37.44,-10.27);
    pos[7].assign(-38.62,-17.45);
    pos[8].assign(-38.11,-1.9);
    pos[9].assign(-31.1,-17.54);
    pos[10].assign(-29.24,-9.25);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 19)
    {
    pos[2].assign(-42.42,-16.44);
    pos[3].assign(-42.25,-8.66);
    pos[4].assign(-42.08,-1.48);
    pos[5].assign(-41.66,5.03);
    pos[6].assign(-33.21,-11.03);
    pos[7].assign(-35.07,-18.04);
    pos[8].assign(-35.66,-2.75);
    pos[9].assign(-27.63,-17.45);
    pos[10].assign(-25.77,-6.89);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 20)
    {
    pos[2].assign(-40.23,-16.44);
    pos[3].assign(-40.06,-8.32);
    pos[4].assign(-40.39,-1.56);
    pos[5].assign(-40.56,5.28);
    pos[6].assign(-29.07,-11.96);
    pos[7].assign(-33.8,-18.8);
    pos[8].assign(-33.72,-2.66);
    pos[9].assign(-23.92,-17.54);
    pos[10].assign(-23.24,-4.61);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 21)
    {
    pos[2].assign(-38.28,-16.52);
    pos[3].assign(-38.2,-8.32);
    pos[4].assign(-38.54,-1.65);
    pos[5].assign(-38.87,5.03);
    pos[6].assign(-25.61,-12.46);
    pos[7].assign(-30.17,-19.82);
    pos[8].assign(-31.61,-3.25);
    pos[9].assign(-18.85,-18.21);
    pos[10].assign(-20.45,-6.3);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 22)
    {
    pos[2].assign(-35.83,-16.44);
    pos[3].assign(-35.49,-8.58);
    pos[4].assign(-35.75,-1.65);
    pos[5].assign(-35.66,4.61);
    pos[6].assign(-23.49,-12.04);
    pos[7].assign(-25.94,-19.9);
    pos[8].assign(-28.39,-3.51);
    pos[9].assign(-14.28,-18.55);
    pos[10].assign(-17.92,-7.56);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 23)
    {
    pos[2].assign(-32.96,-17.7);
    pos[3].assign(-33.38,-10.1);
    pos[4].assign(-33.3,-2.92);
    pos[5].assign(-33.46,4.61);
    pos[6].assign(-19.77,-10.86);
    pos[7].assign(-23.49,-18.72);
    pos[8].assign(-25.69,-2.32);
    pos[9].assign(-10.82,-17.87);
    pos[10].assign(-13.44,-6.63);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 24)
    {
    pos[2].assign(-29.83,-19.99);
    pos[3].assign(-29.92,-12.04);
    pos[4].assign(-30.17,-4.69);
    pos[5].assign(-30.51,2.49);
    pos[6].assign(-16.06,-12.46);
    pos[7].assign(-19.94,-19.82);
    pos[8].assign(-20.62,-3.42);
    pos[9].assign(-7.86,-16.77);
    pos[10].assign(-9.55,-7.23);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 25)
    {
    pos[2].assign(-27.46,-21.59);
    pos[3].assign(-27.72,-13.56);
    pos[4].assign(-27.55,-4.86);
    pos[5].assign(-27.55,2.83);
    pos[6].assign(-12,-13.82);
    pos[7].assign(-16.14,-20.49);
    pos[8].assign(-15.97,-4.1);
    pos[9].assign(-2.96,-14.32);
    pos[10].assign(-5.66,-4.01);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 26)
    {
    pos[2].assign(-19.52,-22.44);
    pos[3].assign(-19.69,-13.06);
    pos[4].assign(-19.77,-3.76);
    pos[5].assign(-20.37,5.79);
    pos[6].assign(-4.14,-13.56);
    pos[7].assign(-9.04,-21);
    pos[8].assign(-9.46,-4.18);
    pos[9].assign(7.61,-13.48);
    pos[10].assign(3.46,-3.59);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 27)
    {
    pos[2].assign(-11.75,-21.93);
    pos[3].assign(-11.92,-12.8);
    pos[4].assign(-12.42,-3.08);
    pos[5].assign(-12.68,4.94);
    pos[6].assign(6,-13.56);
    pos[7].assign(0.08,-2.75);
    pos[8].assign(10.9,-3.17);
    pos[9].assign(18.85,-10.69);
    pos[10].assign(1.27,-22.52);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 28)
    {
    pos[2].assign(-4.9,-21.76);
    pos[3].assign(-5.66,-12.63);
    pos[4].assign(-5.92,-3.25);
    pos[5].assign(-6.42,5.96);
    pos[6].assign(9.46,-13.39);
    pos[7].assign(6,-2.15);
    pos[8].assign(17.83,-3);
    pos[9].assign(28.82,-10.44);
    pos[10].assign(15.04,-22.61);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 29)
    {
    pos[2].assign(-0.59,-21.08);
    pos[3].assign(-0.59,-12.04);
    pos[4].assign(-1.01,-3);
    pos[5].assign(-0.25,6.46);
    pos[6].assign(12.85,-13.39);
    pos[7].assign(11.66,0.89);
    pos[8].assign(24.59,-4.44);
    pos[9].assign(35.41,-12.04);
    pos[10].assign(22.65,-23.11);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 30)
    {
    pos[2].assign(-0.59,-21.08);
    pos[3].assign(-0.59,-12.04);
    pos[4].assign(-1.01,-3);
    pos[5].assign(-0.25,6.46);
    pos[6].assign(18.17,-13.65);
    pos[7].assign(15.8,0.46);
    pos[8].assign(28.39,-5.37);
    pos[9].assign(40.48,-10.18);
    pos[10].assign(30.59,-23.03);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 31)
    {
    pos[2].assign(-50.03,-9.93);
    pos[3].assign(-44.87,-7.39);
    pos[4].assign(-45.38,-0.21);
    pos[5].assign(-41.66,5.79);
    pos[6].assign(-37.52,-9);
    pos[7].assign(-43.35,-13.9);
    pos[8].assign(-38.79,-1.82);
    pos[9].assign(-35.07,-16.52);
    pos[10].assign(-31.52,-4.52);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 32)
    {
    pos[2].assign(-50.03,-9.93);
    pos[3].assign(-44.87,-7.39);
    pos[4].assign(-45.38,-0.21);
    pos[5].assign(-41.66,5.79);
    pos[6].assign(-37.52,-9);
    pos[7].assign(-41.92,-12.89);
    pos[8].assign(-38.79,-1.82);
    pos[9].assign(-35.07,-16.52);
    pos[10].assign(-31.52,-4.52);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 33)
    {
    pos[2].assign(-47.75,-9.85);
    pos[3].assign(-42.34,-7.06);
    pos[4].assign(-45.38,-0.21);
    pos[5].assign(-41.66,5.79);
    pos[6].assign(-36.34,-9.76);
    pos[7].assign(-43.01,-12.8);
    pos[8].assign(-37.44,-0.63);
    pos[9].assign(-35.07,-16.52);
    pos[10].assign(-31.52,-4.52);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 34)
    {
    pos[2].assign(-47.75,-9.85);
    pos[3].assign(-42.34,-7.06);
    pos[4].assign(-45.38,-0.21);
    pos[5].assign(-41.66,5.79);
    pos[6].assign(-36.17,-10.69);
    pos[7].assign(-42,-15.25);
    pos[8].assign(-37.44,-0.63);
    pos[9].assign(-30.76,-15.76);
    pos[10].assign(-29.75,-7.31);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 35)
    {
    pos[2].assign(-45.46,-11.11);
    pos[3].assign(-40.99,-6.8);
    pos[4].assign(-44.54,-1.14);
    pos[5].assign(-40.31,6.38);
    pos[6].assign(-35.41,-11.03);
    pos[7].assign(-39.8,-15.68);
    pos[8].assign(-34.9,-0.55);
    pos[9].assign(-25.18,-15.17);
    pos[10].assign(-29.75,-7.31);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 36)
    {
    pos[2].assign(-42.51,-11.54);
    pos[3].assign(-37.94,-6.63);
    pos[4].assign(-41.32,-0.72);
    pos[5].assign(-38.45,6.21);
    pos[6].assign(-31.52,-10.94);
    pos[7].assign(-35.83,-15.85);
    pos[8].assign(-29.75,-0.55);
    pos[9].assign(-20.11,-15.25);
    pos[10].assign(-23.24,-7.23);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 37)
    {
    pos[2].assign(-36.93,-12.63);
    pos[3].assign(-37.1,-6.63);
    pos[4].assign(-37.27,0.13);
    pos[5].assign(-37.27,6.3);
    pos[6].assign(-25.69,-8.66);
    pos[7].assign(-30.85,-15.76);
    pos[8].assign(-29.75,-0.55);
    pos[9].assign(-15.3,-14.41);
    pos[10].assign(-16.56,-5.45);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 38)
    {
    pos[2].assign(-32.62,-12.63);
    pos[3].assign(-33.13,-5.87);
    pos[4].assign(-33.38,0.46);
    pos[5].assign(-33.89,6.72);
    pos[6].assign(-21.55,-8.75);
    pos[7].assign(-26.37,-16.27);
    pos[8].assign(-23.66,-0.21);
    pos[9].assign(-10.9,-14.41);
    pos[10].assign(-11.83,-5.7);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 39)
    {
    pos[2].assign(-28.73,-12.63);
    pos[3].assign(-29.15,-5.2);
    pos[4].assign(-29.49,0.8);
    pos[5].assign(-29.58,8.15);
    pos[6].assign(-16.99,-8.92);
    pos[7].assign(-21.63,-18.63);
    pos[8].assign(-21.21,1.23);
    pos[9].assign(-3.89,-17.54);
    pos[10].assign(-6.08,-3.59);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 40)
    {
    pos[2].assign(-24.93,-12.63);
    pos[3].assign(-25.61,-5.2);
    pos[4].assign(-25.77,1.56);
    pos[5].assign(-25.77,7.73);
    pos[6].assign(-11.83,-8.83);
    pos[7].assign(-16.99,-18.89);
    pos[8].assign(-15.89,1.9);
    pos[9].assign(2.03,-19.14);
    pos[10].assign(-0.08,-4.77);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 41)
    {
    pos[2].assign(-18.93,-12.97);
    pos[3].assign(-19.86,-5.11);
    pos[4].assign(-20.28,2.32);
    pos[5].assign(-20.37,9.34);
    pos[6].assign(-6,-10.01);
    pos[7].assign(-11.07,-18.97);
    pos[8].assign(-10.99,1.56);
    pos[9].assign(11.24,-19.56);
    pos[10].assign(9.72,-4.86);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 42)
    {
    pos[2].assign(-9.97,-13.14);
    pos[3].assign(-10.48,-4.27);
    pos[4].assign(-10.48,3.85);
    pos[5].assign(-10.9,11.96);
    pos[6].assign(3.8,-9.51);
    pos[7].assign(-0.93,-18.8);
    pos[8].assign(-1.69,1.73);
    pos[9].assign(22.14,-20.66);
    pos[10].assign(20.87,-4.69);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 43)
    {
    pos[2].assign(-3.13,-13.06);
    pos[3].assign(-3.63,-4.18);
    pos[4].assign(-3.72,3.51);
    pos[5].assign(-3.8,12.13);
    pos[6].assign(15.13,-10.94);
    pos[7].assign(7.27,-19.14);
    pos[8].assign(6.76,1.06);
    pos[9].assign(32.54,-20.66);
    pos[10].assign(25.27,-4.77);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 44)
    {
    pos[2].assign(0.51,-13.06);
    pos[3].assign(0.42,-3.85);
    pos[4].assign(0.59,3.85);
    pos[5].assign(0.76,12.04);
    pos[6].assign(21.3,-11.62);
    pos[7].assign(12.42,-19.31);
    pos[8].assign(15.04,0.8);
    pos[9].assign(32.11,-21.59);
    pos[10].assign(31.01,-3.51);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 45)
    {
    pos[2].assign(3.89,-13.48);
    pos[3].assign(3.97,-3.17);
    pos[4].assign(3.72,4.61);
    pos[5].assign(3.63,12.46);
    pos[6].assign(26.28,-12.13);
    pos[7].assign(16.9,-19.14);
    pos[8].assign(19.44,1.73);
    pos[9].assign(35.66,-21.59);
    pos[10].assign(35.75,-3.34);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 46)
    {
    pos[2].assign(-50.96,-7.06);
    pos[3].assign(-46.31,-8.58);
    pos[4].assign(-46.48,-3.34);
    pos[5].assign(-49.01,3.76);
    pos[6].assign(-41.24,-6.89);
    pos[7].assign(-42.51,-13.99);
    pos[8].assign(-42.34,2.15);
    pos[9].assign(-36.25,-11.62);
    pos[10].assign(-36.59,-1.73);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 47)
    {
    pos[2].assign(-46.31,-11.37);
    pos[3].assign(-48.25,-6.04);
    pos[4].assign(-46.06,-1.9);
    pos[5].assign(-48.76,4.86);
    pos[6].assign(-41.24,-6.89);
    pos[7].assign(-40.14,-14.24);
    pos[8].assign(-40.65,1.48);
    pos[9].assign(-34.99,-10.44);
    pos[10].assign(-35.07,-2.49);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 48)
    {
    pos[2].assign(-46.31,-11.37);
    pos[3].assign(-46.9,-5.7);
    pos[4].assign(-46.99,-0.72);
    pos[5].assign(-48.76,4.86);
    pos[6].assign(-40.99,-4.94);
    pos[7].assign(-40.14,-14.24);
    pos[8].assign(-38.28,2.24);
    pos[9].assign(-34.99,-10.44);
    pos[10].assign(-34.06,-3.25);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 49)
    {
    pos[2].assign(-45.13,-10.35);
    pos[3].assign(-45.04,-5.62);
    pos[4].assign(-44.96,0.21);
    pos[5].assign(-44.87,6.21);
    pos[6].assign(-40.23,-7.31);
    pos[7].assign(-40.99,-14.58);
    pos[8].assign(-38.79,0.21);
    pos[9].assign(-32.87,-15.08);
    pos[10].assign(-32.03,-0.13);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 50)
    {
    pos[2].assign(-41.75,-12.04);
    pos[3].assign(-41.32,-5.2);
    pos[4].assign(-41.49,0.63);
    pos[5].assign(-41.49,6.46);
    pos[6].assign(-36.51,-8.66);
    pos[7].assign(-36.34,-17.37);
    pos[8].assign(-35.49,-0.63);
    pos[9].assign(-28.56,-15.93);
    pos[10].assign(-28.48,-1.14);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 51)
    {
    pos[2].assign(-39.3,-12.89);
    pos[3].assign(-39.21,-5.62);
    pos[4].assign(-39.21,0.63);
    pos[5].assign(-38.87,6.46);
    pos[6].assign(-33.63,-9.25);
    pos[7].assign(-33.63,-16.86);
    pos[8].assign(-32.37,-0.13);
    pos[9].assign(-24.68,-15.76);
    pos[10].assign(-23.92,-1.14);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 52)
    {
    pos[2].assign(-36.34,-12.63);
    pos[3].assign(-36.59,-6.04);
    pos[4].assign(-36.34,0.3);
    pos[5].assign(-35.92,5.96);
    pos[6].assign(-29.15,-9);
    pos[7].assign(-29.15,-16.61);
    pos[8].assign(-29.07,0.38);
    pos[9].assign(-18.85,-17.11);
    pos[10].assign(-19.1,-0.21);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 53)
    {
    pos[2].assign(-33.89,-12.04);
    pos[3].assign(-33.8,-6.21);
    pos[4].assign(-33.3,-0.38);
    pos[5].assign(-33.38,5.2);
    pos[6].assign(-24.17,-9);
    pos[7].assign(-24.68,-17.79);
    pos[8].assign(-23.24,2.32);
    pos[9].assign(-12.42,-16.94);
    pos[10].assign(-12.08,-0.13);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 54)
    {
    pos[2].assign(-29.83,-12.04);
    pos[3].assign(-30.08,-6.3);
    pos[4].assign(-29.83,-0.04);
    pos[5].assign(-30.08,7.31);
    pos[6].assign(-20.87,-9.08);
    pos[7].assign(-18.85,-16.69);
    pos[8].assign(-17.75,1.56);
    pos[9].assign(-5.83,-16.94);
    pos[10].assign(-5.49,0.13);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 55)
    {
    pos[2].assign(-27.8,-12.13);
    pos[3].assign(-28.31,-6.13);
    pos[4].assign(-28.31,-0.21);
    pos[5].assign(-28.31,6.72);
    pos[6].assign(-14.87,-8.83);
    pos[7].assign(-18.85,-16.69);
    pos[8].assign(-17.75,1.56);
    pos[9].assign(-5.83,-16.94);
    pos[10].assign(-5.49,0.13);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 56)
    {
    pos[2].assign(-20.87,-12.97);
    pos[3].assign(-21.13,-6.55);
    pos[4].assign(-21.3,0.46);
    pos[5].assign(-21.46,7.14);
    pos[6].assign(-6.34,-8.41);
    pos[7].assign(-11.75,-17.37);
    pos[8].assign(-12.17,2.24);
    pos[9].assign(1.27,-17.7);
    pos[10].assign(1.44,0.21);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 57)
    {
    pos[2].assign(-12.93,-12.55);
    pos[3].assign(-12.93,-5.7);
    pos[4].assign(-12.93,1.48);
    pos[5].assign(-13.18,8.32);
    pos[6].assign(2.79,-8.58);
    pos[7].assign(-2.62,-17.7);
    pos[8].assign(-2.62,2.49);
    pos[9].assign(9.89,-16.86);
    pos[10].assign(9.63,0.97);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 58)
    {
    pos[2].assign(-2.96,-13.31);
    pos[3].assign(-3.3,-4.18);
    pos[4].assign(-3.21,2.49);
    pos[5].assign(-2.87,9.51);
    pos[6].assign(12.08,-8.75);
    pos[7].assign(6.08,-17.7);
    pos[8].assign(7.69,2.83);
    pos[9].assign(20.28,-17.79);
    pos[10].assign(21.46,1.82);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 59)
    {
    pos[2].assign(2.11,-13.23);
    pos[3].assign(2.7,-3.76);
    pos[4].assign(2.45,5.03);
    pos[5].assign(2.7,12.3);
    pos[6].assign(21.21,-9.08);
    pos[7].assign(13.1,-15.68);
    pos[8].assign(13.94,1.39);
    pos[9].assign(28.23,-18.72);
    pos[10].assign(29.24,-0.55);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 60)
    {
    pos[2].assign(3.89,-13.48);
    pos[3].assign(3.97,-3.17);
    pos[4].assign(3.72,4.61);
    pos[5].assign(3.63,12.46);
    pos[6].assign(26.62,-8.75);
    pos[7].assign(17.83,-15.51);
    pos[8].assign(18.42,1.82);
    pos[9].assign(35.75,-18.46);
    pos[10].assign(36.25,-0.55);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 61)
    {
    pos[2].assign(-50.7,-6.13);
    pos[3].assign(-44.45,-6.97);
    pos[4].assign(-44.96,1.9);
    pos[5].assign(-50.7,4.61);
    pos[6].assign(-41.24,-3.25);
    pos[7].assign(-42.76,-13.14);
    pos[8].assign(-40.99,6.72);
    pos[9].assign(-35.92,-8.75);
    pos[10].assign(-35.75,0.55);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 62)
    {
    pos[2].assign(-50.7,-6.13);
    pos[3].assign(-44.45,-6.97);
    pos[4].assign(-44.96,1.9);
    pos[5].assign(-50.7,4.61);
    pos[6].assign(-41.24,-3.25);
    pos[7].assign(-42.76,-13.14);
    pos[8].assign(-40.99,6.72);
    pos[9].assign(-35.92,-8.75);
    pos[10].assign(-35.75,0.55);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 63)
    {
    pos[2].assign(-44.62,-3.42);
    pos[3].assign(-44.45,1.48);
    pos[4].assign(-47.49,-8.24);
    pos[5].assign(-47.92,6.63);
    pos[6].assign(-40.31,4.44);
    pos[7].assign(-40.56,-10.94);
    pos[8].assign(-34.65,2.32);
    pos[9].assign(-34.39,-9.17);
    pos[10].assign(-33.8,-4.01);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 64)
    {
    pos[2].assign(-44.62,-3.42);
    pos[3].assign(-44.45,1.48);
    pos[4].assign(-44.2,-8.24);
    pos[5].assign(-44.79,5.79);
    pos[6].assign(-39.55,0.89);
    pos[7].assign(-39.46,-7.14);
    pos[8].assign(-38.28,7.9);
    pos[9].assign(-36.51,-11.37);
    pos[10].assign(-33.3,2.15);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 65)
    {
    pos[2].assign(-44.03,-3.34);
    pos[3].assign(-44.79,1.48);
    pos[4].assign(-43.94,-8.66);
    pos[5].assign(-44.79,5.79);
    pos[6].assign(-38.62,-3.08);
    pos[7].assign(-35.49,-8.24);
    pos[8].assign(-35.41,1.56);
    pos[9].assign(-30.17,-12.8);
    pos[10].assign(-31.35,7.06);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 66)
    {
    pos[2].assign(-42.76,-2.83);
    pos[3].assign(-42.93,1.82);
    pos[4].assign(-42.59,-7.56);
    pos[5].assign(-43.1,6.3);
    pos[6].assign(-33.63,-3.17);
    pos[7].assign(-35.41,-9.42);
    pos[8].assign(-35.66,2.75);
    pos[9].assign(-30.17,-12.8);
    pos[10].assign(-31.35,7.06);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 67)
    {
    pos[2].assign(-39.97,-2.75);
    pos[3].assign(-40.39,1.82);
    pos[4].assign(-39.8,-7.65);
    pos[5].assign(-40.73,7.06);
    pos[6].assign(-30.93,-3.17);
    pos[7].assign(-31.27,-11.37);
    pos[8].assign(-34.14,4.18);
    pos[9].assign(-23.58,-12.89);
    pos[10].assign(-24.76,7.39);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 68)
    {
    pos[2].assign(-34.39,-1.56);
    pos[3].assign(-34.73,3.25);
    pos[4].assign(-33.72,-6.97);
    pos[5].assign(-34.73,8.58);
    pos[6].assign(-25.01,-2.49);
    pos[7].assign(-27.55,-10.77);
    pos[8].assign(-27.89,4.86);
    pos[9].assign(-17.49,-13.14);
    pos[10].assign(-17.58,7.65);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 69)
    {
    pos[2].assign(-30.25,-1.39);
    pos[3].assign(-30.34,3.42);
    pos[4].assign(-30.17,-6.89);
    pos[5].assign(-30.51,9.42);
    pos[6].assign(-21.04,-2.75);
    pos[7].assign(-22.31,-11.2);
    pos[8].assign(-21.63,6.55);
    pos[9].assign(-10.99,-12.89);
    pos[10].assign(-12,7.99);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 70)
    {
    pos[2].assign(-26.2,-1.82);
    pos[3].assign(-26.28,3.93);
    pos[4].assign(-25.77,-6.97);
    pos[5].assign(-26.03,8.49);
    pos[6].assign(-14.11,-2.24);
    pos[7].assign(-16.48,-11.54);
    pos[8].assign(-16.23,6.46);
    pos[9].assign(-5.07,-12.97);
    pos[10].assign(-6.17,8.49);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 71)
    {
    pos[2].assign(-22.39,-1.9);
    pos[3].assign(-22.56,3.51);
    pos[4].assign(-21.89,-7.31);
    pos[5].assign(-22.48,9.42);
    pos[6].assign(-8.2,-2.58);
    pos[7].assign(-12.93,-11.2);
    pos[8].assign(-12.51,7.31);
    pos[9].assign(2.28,-12.8);
    pos[10].assign(2.11,9.25);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 72)
    {
    pos[2].assign(-15.63,-1.31);
    pos[3].assign(-15.55,3.51);
    pos[4].assign(-15.13,-7.14);
    pos[5].assign(-15.46,9.42);
    pos[6].assign(2.2,-3.08);
    pos[7].assign(-3.97,-12.13);
    pos[8].assign(-4.9,7.14);
    pos[9].assign(12.59,-12.55);
    pos[10].assign(12.08,9.34);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 73)
    {
    pos[2].assign(-12.08,-1.65);
    pos[3].assign(-12.17,3.68);
    pos[4].assign(-11.75,-7.14);
    pos[5].assign(-12.42,10.44);
    pos[6].assign(9.89,-3);
    pos[7].assign(0.34,-11.87);
    pos[8].assign(0.59,8.58);
    pos[9].assign(22.39,-12.8);
    pos[10].assign(22.9,9.17);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 74)
    {
    pos[2].assign(-4.23,-0.97);
    pos[3].assign(-4.73,4.86);
    pos[4].assign(-3.55,-7.06);
    pos[5].assign(-5.32,10.86);
    pos[6].assign(20.7,-2.66);
    pos[7].assign(11.83,-12.38);
    pos[8].assign(11.15,9.08);
    pos[9].assign(31.1,-12.97);
    pos[10].assign(32.2,8.83);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 75)
    {
    pos[2].assign(1.18,-1.31);
    pos[3].assign(1.27,4.77);
    pos[4].assign(1.35,-7.23);
    pos[5].assign(1.35,10.94);
    pos[6].assign(29.07,-2.41);
    pos[7].assign(21.89,-11.79);
    pos[8].assign(22.73,10.27);
    pos[9].assign(40.99,-12.72);
    pos[10].assign(42.08,9.51);
    pos[11].assign(47.41,28.44);
    }
    
    
    
    else if(point == 76)
    {
    pos[2].assign(-50.7,6.13);
    pos[3].assign(-44.45,6.97);
    pos[4].assign(-44.96,-1.9);
    pos[5].assign(-50.7,-4.61);
    pos[6].assign(-41.24,3.25);
    pos[7].assign(-42.76,13.14);
    pos[8].assign(-40.99,-6.72);
    pos[9].assign(-35.92,8.75);
    pos[10].assign(-35.75,0.55);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 77)
    {
    pos[2].assign(-50.7,6.13);
    pos[3].assign(-44.45,6.97);
    pos[4].assign(-44.96,-1.9);
    pos[5].assign(-50.7,-4.61);
    pos[6].assign(-41.24,3.25);
    pos[7].assign(-42.76,13.14);
    pos[8].assign(-40.99,-6.72);
    pos[9].assign(-35.92,8.75);
    pos[10].assign(-35.75,-0.55);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 78)
    {
    pos[2].assign(-44.62,3.42);
    pos[3].assign(-44.45,-1.48);
    pos[4].assign(-47.49,8.24);
    pos[5].assign(-47.92,-6.63);
    pos[6].assign(-40.31,-4.44);
    pos[7].assign(-40.56,10.94);
    pos[8].assign(-34.65,-2.32);
    pos[9].assign(-34.39,9.17);
    pos[10].assign(-33.8,4.01);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 79)
    {
    pos[2].assign(-44.62,3.42);
    pos[3].assign(-44.45,-1.48);
    pos[4].assign(-44.2,8.24);
    pos[5].assign(-44.79,-5.79);
    pos[6].assign(-39.55,-0.89);
    pos[7].assign(-39.46,7.14);
    pos[8].assign(-38.28,-7.9);
    pos[9].assign(-36.51,11.37);
    pos[10].assign(-33.3,-2.15);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 80)
    {
    pos[2].assign(-44.03,3.34);
    pos[3].assign(-44.79,-1.48);
    pos[4].assign(-43.94,8.66);
    pos[5].assign(-44.79,-5.79);
    pos[6].assign(-38.62,3.08);
    pos[7].assign(-35.49,8.24);
    pos[8].assign(-35.41,-1.56);
    pos[9].assign(-30.17,12.8);
    pos[10].assign(-31.35,-7.06);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 81)
    {
    pos[2].assign(-42.76,2.83);
    pos[3].assign(-42.93,-1.82);
    pos[4].assign(-42.59,7.56);
    pos[5].assign(-43.1,-6.3);
    pos[6].assign(-33.63,3.17);
    pos[7].assign(-35.41,9.42);
    pos[8].assign(-35.66,-2.75);
    pos[9].assign(-30.17,12.8);
    pos[10].assign(-31.35,-7.06);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 82)
    {
    pos[2].assign(-39.97,2.75);
    pos[3].assign(-40.39,-1.82);
    pos[4].assign(-39.8,7.65);
    pos[5].assign(-40.73,-7.06);
    pos[6].assign(-30.93,3.17);
    pos[7].assign(-31.27,11.37);
    pos[8].assign(-34.14,-4.18);
    pos[9].assign(-23.58,12.89);
    pos[10].assign(-24.76,-7.39);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 83)
    {
    pos[2].assign(-34.39,1.56);
    pos[3].assign(-34.73,-3.25);
    pos[4].assign(-33.72,6.97);
    pos[5].assign(-34.73,-8.58);
    pos[6].assign(-25.01,2.49);
    pos[7].assign(-27.55,10.77);
    pos[8].assign(-27.89,-4.86);
    pos[9].assign(-17.49,13.14);
    pos[10].assign(-17.58,-7.65);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 84)
    {
    pos[2].assign(-30.25,1.39);
    pos[3].assign(-30.34,-3.42);
    pos[4].assign(-30.17,6.89);
    pos[5].assign(-30.51,-9.42);
    pos[6].assign(-21.04,2.75);
    pos[7].assign(-22.31,11.2);
    pos[8].assign(-21.63,-6.55);
    pos[9].assign(-10.99,12.89);
    pos[10].assign(-12,-7.99);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 85)
    {
    pos[2].assign(-26.2,1.82);
    pos[3].assign(-26.28,-3.93);
    pos[4].assign(-25.77,6.97);
    pos[5].assign(-26.03,-8.49);
    pos[6].assign(-14.11,2.24);
    pos[7].assign(-16.48,11.54);
    pos[8].assign(-16.23,-6.46);
    pos[9].assign(-5.07,12.97);
    pos[10].assign(-6.17,-8.49);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 86)
    {
    pos[2].assign(-22.39,1.9);
    pos[3].assign(-22.56,-3.51);
    pos[4].assign(-21.89,7.31);
    pos[5].assign(-22.48,-9.42);
    pos[6].assign(-8.2,2.58);
    pos[7].assign(-12.93,11.2);
    pos[8].assign(-12.51,-7.31);
    pos[9].assign(2.28,12.8);
    pos[10].assign(2.11,-9.25);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 87)
    {
    pos[2].assign(-15.63,1.31);
    pos[3].assign(-15.55,-3.51);
    pos[4].assign(-15.13,7.14);
    pos[5].assign(-15.46,-9.42);
    pos[6].assign(2.2,3.08);
    pos[7].assign(-3.97,12.13);
    pos[8].assign(-4.9,-7.14);
    pos[9].assign(12.59,12.55);
    pos[10].assign(12.08,-9.34);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 88)
    {
    pos[2].assign(-12.08,1.65);
    pos[3].assign(-12.17,-3.68);
    pos[4].assign(-11.75,7.14);
    pos[5].assign(-12.42,-10.44);
    pos[6].assign(9.89,3);
    pos[7].assign(0.34,11.87);
    pos[8].assign(0.59,-8.58);
    pos[9].assign(22.39,12.8);
    pos[10].assign(22.9,-9.17);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 89)
    {
    pos[2].assign(-4.23,0.97);
    pos[3].assign(-4.73,-4.86);
    pos[4].assign(-3.55,7.06);
    pos[5].assign(-5.32,-10.86);
    pos[6].assign(20.7,2.66);
    pos[7].assign(11.83,12.38);
    pos[8].assign(11.15,-9.08);
    pos[9].assign(31.1,12.97);
    pos[10].assign(32.2,-8.83);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 90)
    {
    pos[2].assign(1.18,1.31);
    pos[3].assign(1.27,-4.77);
    pos[4].assign(1.35,7.23);
    pos[5].assign(1.35,-10.94);
    pos[6].assign(29.07,2.41);
    pos[7].assign(21.89,11.79);
    pos[8].assign(22.73,-10.27);
    pos[9].assign(40.99,12.72);
    pos[10].assign(42.08,-9.51);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 91)
    {
    pos[2].assign(-50.96,7.06);
    pos[3].assign(-46.31,8.58);
    pos[4].assign(-46.48,3.34);
    pos[5].assign(-49.01,-3.76);
    pos[6].assign(-41.24,6.89);
    pos[7].assign(-42.51,13.99);
    pos[8].assign(-42.34,-2.15);
    pos[9].assign(-36.25,11.62);
    pos[10].assign(-36.59,1.73);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 92)
    {
    pos[2].assign(-46.31,11.37);
    pos[3].assign(-48.25,6.04);
    pos[4].assign(-46.06,1.9);
    pos[5].assign(-48.76,-4.86);
    pos[6].assign(-41.24,6.89);
    pos[7].assign(-40.14,14.24);
    pos[8].assign(-40.65,-1.48);
    pos[9].assign(-34.99,10.44);
    pos[10].assign(-35.07,2.49);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 93)
    {
    pos[2].assign(-46.31,11.37);
    pos[3].assign(-46.9,5.7);
    pos[4].assign(-46.99,0.72);
    pos[5].assign(-48.76,-4.86);
    pos[6].assign(-40.99,4.94);
    pos[7].assign(-40.14,14.24);
    pos[8].assign(-38.28,-2.24);
    pos[9].assign(-34.99,10.44);
    pos[10].assign(-34.06,3.25);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 94)
    {
    pos[2].assign(-45.13,10.35);
    pos[3].assign(-45.04,5.62);
    pos[4].assign(-44.96,-0.21);
    pos[5].assign(-44.87,-6.21);
    pos[6].assign(-40.23,7.31);
    pos[7].assign(-40.99,14.58);
    pos[8].assign(-38.79,-0.21);
    pos[9].assign(-32.87,15.08);
    pos[10].assign(-32.03,0.13);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 95)
    {
    pos[2].assign(-41.75,12.04);
    pos[3].assign(-41.32,5.2);
    pos[4].assign(-41.49,-0.63);
    pos[5].assign(-41.49,-6.46);
    pos[6].assign(-36.51,8.66);
    pos[7].assign(-36.34,17.37);
    pos[8].assign(-35.49,0.63);
    pos[9].assign(-28.56,15.93);
    pos[10].assign(-28.48,1.14);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 96)
    {
    pos[2].assign(-39.3,12.89);
    pos[3].assign(-39.21,5.62);
    pos[4].assign(-39.21,-0.63);
    pos[5].assign(-38.87,-6.46);
    pos[6].assign(-33.63,9.25);
    pos[7].assign(-33.63,16.86);
    pos[8].assign(-32.37,0.13);
    pos[9].assign(-24.68,15.76);
    pos[10].assign(-23.92,1.14);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 97)
    {
    pos[2].assign(-36.34,12.63);
    pos[3].assign(-36.59,6.04);
    pos[4].assign(-36.34,-0.3);
    pos[5].assign(-35.92,-5.96);
    pos[6].assign(-29.15,9);
    pos[7].assign(-29.15,16.61);
    pos[8].assign(-29.07,-0.38);
    pos[9].assign(-18.85,17.11);
    pos[10].assign(-19.1,0.21);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 98)
    {
    pos[2].assign(-33.89,12.04);
    pos[3].assign(-33.8,6.21);
    pos[4].assign(-33.3,0.38);
    pos[5].assign(-33.38,-5.2);
    pos[6].assign(-24.17,9);
    pos[7].assign(-24.68,17.79);
    pos[8].assign(-23.24,-2.32);
    pos[9].assign(-12.42,16.94);
    pos[10].assign(-12.08,0.13);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 99)
    {
    pos[2].assign(-29.83,12.04);
    pos[3].assign(-30.08,6.3);
    pos[4].assign(-29.83,0.04);
    pos[5].assign(-30.08,-7.31);
    pos[6].assign(-20.87,9.08);
    pos[7].assign(-18.85,16.69);
    pos[8].assign(-17.75,-1.56);
    pos[9].assign(-5.83,16.94);
    pos[10].assign(-5.49,-0.13);
    pos[11].assign(47.41,28.44);
    }
    else if(point == 100)
    {
    pos[2].assign(-27.8,12.13);
    pos[3].assign(-28.31,6.13);
    pos[4].assign(-28.31,0.21);
    pos[5].assign(-28.31,-6.72);
    pos[6].assign(-14.87,8.83);
    pos[7].assign(-18.85,16.69);
    pos[8].assign(-17.75,-1.56);
    pos[9].assign(-5.83,16.94);
    pos[10].assign(-5.49,-0.13);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 101)
    {
    pos[2].assign(-20.87,12.97);
    pos[3].assign(-21.13,6.55);
    pos[4].assign(-21.3,-0.46);
    pos[5].assign(-21.46,-7.14);
    pos[6].assign(-6.34,8.41);
    pos[7].assign(-11.75,17.37);
    pos[8].assign(-12.17,-2.24);
    pos[9].assign(1.27,17.7);
    pos[10].assign(1.44,-0.21);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 102)
    {
    pos[2].assign(-12.93,12.55);
    pos[3].assign(-12.93,5.7);
    pos[4].assign(-12.93,-1.48);
    pos[5].assign(-13.18,-8.32);
    pos[6].assign(2.79,8.58);
    pos[7].assign(-2.62,17.7);
    pos[8].assign(-2.62,-2.49);
    pos[9].assign(9.89,16.86);
    pos[10].assign(9.63,-0.97);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 103)
    {
    pos[2].assign(-2.96,13.31);
    pos[3].assign(-3.3,4.18);
    pos[4].assign(-3.21,-2.49);
    pos[5].assign(-2.87,-9.51);
    pos[6].assign(12.08,8.75);
    pos[7].assign(6.08,17.7);
    pos[8].assign(7.69,-2.83);
    pos[9].assign(20.28,17.79);
    pos[10].assign(21.46,-1.82);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 104)
    {
    pos[2].assign(2.11,13.23);
    pos[3].assign(2.7,3.76);
    pos[4].assign(2.45,-5.03);
    pos[5].assign(2.7,-12.3);
    pos[6].assign(21.21,9.08);
    pos[7].assign(13.1,15.68);
    pos[8].assign(13.94,-1.39);
    pos[9].assign(28.23,18.72);
    pos[10].assign(29.24,0.55);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 105)
    {
    pos[2].assign(3.89,13.48);
    pos[3].assign(3.97,3.17);
    pos[4].assign(3.72,-4.61);
    pos[5].assign(3.63,-12.46);
    pos[6].assign(26.62,8.75);
    pos[7].assign(17.83,15.51);
    pos[8].assign(18.42,-1.82);
    pos[9].assign(35.75,18.46);
    pos[10].assign(36.25,0.55);
    pos[11].assign(47.41,-28.44);
    }
        else if(point == 106)
    {
    pos[2].assign(-50.03,9.93);
    pos[3].assign(-44.87,7.39);
    pos[4].assign(-45.38,0.21);
    pos[5].assign(-41.66,-5.79);
    pos[6].assign(-37.52,9);
    pos[7].assign(-43.35,13.9);
    pos[8].assign(-38.79,1.82);
    pos[9].assign(-35.07,16.52);
    pos[10].assign(-31.52,4.52);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 107)
    {
    pos[2].assign(-50.03,9.93);
    pos[3].assign(-44.87,7.39);
    pos[4].assign(-45.38,0.21);
    pos[5].assign(-41.66,-5.79);
    pos[6].assign(-37.52,9);
    pos[7].assign(-41.92,12.89);
    pos[8].assign(-38.79,1.82);
    pos[9].assign(-35.07,16.52);
    pos[10].assign(-31.52,4.52);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 108)
    {
    pos[2].assign(-47.75,9.85);
    pos[3].assign(-42.34,7.06);
    pos[4].assign(-45.38,0.21);
    pos[5].assign(-41.66,-5.79);
    pos[6].assign(-36.34,9.76);
    pos[7].assign(-43.01,12.8);
    pos[8].assign(-37.44,0.63);
    pos[9].assign(-35.07,16.52);
    pos[10].assign(-31.52,4.52);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 109)
    {
    pos[2].assign(-47.75,9.85);
    pos[3].assign(-42.34,7.06);
    pos[4].assign(-45.38,0.21);
    pos[5].assign(-41.66,-5.79);
    pos[6].assign(-36.17,10.69);
    pos[7].assign(-42,15.25);
    pos[8].assign(-37.44,0.63);
    pos[9].assign(-30.76,15.76);
    pos[10].assign(-29.75,7.31);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 110)
    {
    pos[2].assign(-45.46,11.11);
    pos[3].assign(-40.99,6.8);
    pos[4].assign(-44.54,1.14);
    pos[5].assign(-40.31,-6.38);
    pos[6].assign(-35.41,11.03);
    pos[7].assign(-39.8,15.68);
    pos[8].assign(-34.9,0.55);
    pos[9].assign(-25.18,15.17);
    pos[10].assign(-29.75,7.31);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 111)
    {
    pos[2].assign(-42.51,11.54);
    pos[3].assign(-37.94,6.63);
    pos[4].assign(-41.32,0.72);
    pos[5].assign(-38.45,-6.21);
    pos[6].assign(-31.52,10.94);
    pos[7].assign(-35.83,15.85);
    pos[8].assign(-29.75,0.55);
    pos[9].assign(-20.11,15.25);
    pos[10].assign(-23.24,7.23);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 112)
    {
    pos[2].assign(-36.93,12.63);
    pos[3].assign(-37.1,6.63);
    pos[4].assign(-37.27,-0.13);
    pos[5].assign(-37.27,-6.3);
    pos[6].assign(-25.69,8.66);
    pos[7].assign(-30.85,15.76);
    pos[8].assign(-29.75,0.55);
    pos[9].assign(-15.3,14.41);
    pos[10].assign(-16.56,5.45);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 113)
    {
    pos[2].assign(-32.62,12.63);
    pos[3].assign(-33.13,5.87);
    pos[4].assign(-33.38,-0.46);
    pos[5].assign(-33.89,-6.72);
    pos[6].assign(-21.55,8.75);
    pos[7].assign(-26.37,16.27);
    pos[8].assign(-23.66,0.21);
    pos[9].assign(-10.9,14.41);
    pos[10].assign(-11.83,5.7);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 114)
    {
    pos[2].assign(-28.73,12.63);
    pos[3].assign(-29.15,5.2);
    pos[4].assign(-29.49,-0.8);
    pos[5].assign(-29.58,-8.15);
    pos[6].assign(-16.99,8.92);
    pos[7].assign(-21.63,18.63);
    pos[8].assign(-21.21,-1.23);
    pos[9].assign(-3.89,17.54);
    pos[10].assign(-6.08,3.59);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 115)
    {
    pos[2].assign(-24.93,12.63);
    pos[3].assign(-25.61,5.2);
    pos[4].assign(-25.77,-1.56);
    pos[5].assign(-25.77,-7.73);
    pos[6].assign(-11.83,8.83);
    pos[7].assign(-16.99,18.89);
    pos[8].assign(-15.89,-1.9);
    pos[9].assign(2.03,19.14);
    pos[10].assign(-0.08,4.77);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 116)
    {
    pos[2].assign(-18.93,12.97);
    pos[3].assign(-19.86,5.11);
    pos[4].assign(-20.28,-2.32);
    pos[5].assign(-20.37,-9.34);
    pos[6].assign(-6,10.01);
    pos[7].assign(-11.07,18.97);
    pos[8].assign(-10.99,-1.56);
    pos[9].assign(11.24,19.56);
    pos[10].assign(9.72,4.86);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 117)
    {
    pos[2].assign(-9.97,13.14);
    pos[3].assign(-10.48,4.27);
    pos[4].assign(-10.48,-3.85);
    pos[5].assign(-10.9,-11.96);
    pos[6].assign(3.8,9.51);
    pos[7].assign(-0.93,18.8);
    pos[8].assign(-1.69,-1.73);
    pos[9].assign(22.14,20.66);
    pos[10].assign(20.87,4.69);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 118)
    {
    pos[2].assign(-3.13,13.06);
    pos[3].assign(-3.63,4.18);
    pos[4].assign(-3.72,-3.51);
    pos[5].assign(-3.8,-12.13);
    pos[6].assign(15.13,10.94);
    pos[7].assign(7.27,19.14);
    pos[8].assign(6.76,-1.06);
    pos[9].assign(32.54,20.66);
    pos[10].assign(25.27,4.77);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 119)
    {
    pos[2].assign(0.51,13.06);
    pos[3].assign(0.42,3.85);
    pos[4].assign(0.59,-3.85);
    pos[5].assign(0.76,-12.04);
    pos[6].assign(21.3,11.62);
    pos[7].assign(12.42,19.31);
    pos[8].assign(15.04,-0.8);
    pos[9].assign(32.11,21.59);
    pos[10].assign(31.01,3.51);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 120)
    {
    pos[2].assign(3.89,13.48);
    pos[3].assign(3.97,3.17);
    pos[4].assign(3.72,-4.61);
    pos[5].assign(3.63,-12.46);
    pos[6].assign(26.28,12.13);
    pos[7].assign(16.9,19.14);
    pos[8].assign(19.44,-1.73);
    pos[9].assign(35.66,21.59);
    pos[10].assign(35.75,3.34);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 121)
    {
    pos[2].assign(-49.77,17.11);
    pos[3].assign(-46.23,8.66);
    pos[4].assign(-45.38,0.21);
    pos[5].assign(-41.83,-6.13);
    pos[6].assign(-40.06,10.27);
    pos[7].assign(-42.42,17.37);
    pos[8].assign(-39.21,2.58);
    pos[9].assign(-34.48,20.92);
    pos[10].assign(-33.46,13.23);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 122)
    {
    pos[2].assign(-46.56,17.03);
    pos[3].assign(-46.23,8.66);
    pos[4].assign(-45.38,0.21);
    pos[5].assign(-41.83,-6.13);
    pos[6].assign(-40.06,10.27);
    pos[7].assign(-38.62,17.45);
    pos[8].assign(-39.21,2.58);
    pos[9].assign(-32.96,21.17);
    pos[10].assign(-31.69,11.45);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 123)
    {
    pos[2].assign(-44.62,16.52);
    pos[3].assign(-45.21,8.92);
    pos[4].assign(-44.54,0.63);
    pos[5].assign(-41.83,-6.13);
    pos[6].assign(-37.44,10.27);
    pos[7].assign(-38.62,17.45);
    pos[8].assign(-38.11,1.9);
    pos[9].assign(-31.1,17.54);
    pos[10].assign(-29.24,9.25);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 124)
    {
    pos[2].assign(-42.42,16.44);
    pos[3].assign(-42.25,8.66);
    pos[4].assign(-42.08,1.48);
    pos[5].assign(-41.66,-5.03);
    pos[6].assign(-33.21,11.03);
    pos[7].assign(-35.07,18.04);
    pos[8].assign(-35.66,2.75);
    pos[9].assign(-27.63,17.45);
    pos[10].assign(-25.77,6.89);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 125)
    {
    pos[2].assign(-40.23,16.44);
    pos[3].assign(-40.06,8.32);
    pos[4].assign(-40.39,1.56);
    pos[5].assign(-40.56,-5.28);
    pos[6].assign(-29.07,11.96);
    pos[7].assign(-33.8,18.8);
    pos[8].assign(-33.72,2.66);
    pos[9].assign(-23.92,17.54);
    pos[10].assign(-23.24,4.61);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 126)
    {
    pos[2].assign(-38.28,16.52);
    pos[3].assign(-38.2,8.32);
    pos[4].assign(-38.54,1.65);
    pos[5].assign(-38.87,-5.03);
    pos[6].assign(-25.61,12.46);
    pos[7].assign(-30.17,19.82);
    pos[8].assign(-31.61,3.25);
    pos[9].assign(-18.85,18.21);
    pos[10].assign(-20.45,6.3);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 127)
    {
    pos[2].assign(-35.83,16.44);
    pos[3].assign(-35.49,8.58);
    pos[4].assign(-35.75,1.65);
    pos[5].assign(-35.66,-4.61);
    pos[6].assign(-23.49,12.04);
    pos[7].assign(-25.94,19.9);
    pos[8].assign(-28.39,3.51);
    pos[9].assign(-14.28,18.55);
    pos[10].assign(-17.92,7.56);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 128)
    {
    pos[2].assign(-32.96,17.7);
    pos[3].assign(-33.38,10.1);
    pos[4].assign(-33.3,2.92);
    pos[5].assign(-33.46,-4.61);
    pos[6].assign(-19.77,10.86);
    pos[7].assign(-23.49,18.72);
    pos[8].assign(-25.69,2.32);
    pos[9].assign(-10.82,17.87);
    pos[10].assign(-13.44,6.63);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 129)
    {
    pos[2].assign(-29.83,19.99);
    pos[3].assign(-29.92,12.04);
    pos[4].assign(-30.17,4.69);
    pos[5].assign(-30.51,-2.49);
    pos[6].assign(-16.06,12.46);
    pos[7].assign(-19.94,19.82);
    pos[8].assign(-20.62,3.42);
    pos[9].assign(-7.86,16.77);
    pos[10].assign(-9.55,7.23);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 130)
    {
    pos[2].assign(-27.46,21.59);
    pos[3].assign(-27.72,13.56);
    pos[4].assign(-27.55,4.86);
    pos[5].assign(-27.55,-2.83);
    pos[6].assign(-12,13.82);
    pos[7].assign(-16.14,20.49);
    pos[8].assign(-15.97,4.1);
    pos[9].assign(-2.96,14.32);
    pos[10].assign(-5.66,4.01);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 131)
    {
    pos[2].assign(-19.52,22.44);
    pos[3].assign(-19.69,13.06);
    pos[4].assign(-19.77,3.76);
    pos[5].assign(-20.37,-5.79);
    pos[6].assign(-4.14,13.56);
    pos[7].assign(-9.04,21);
    pos[8].assign(-9.46,4.18);
    pos[9].assign(7.61,13.48);
    pos[10].assign(3.46,3.59);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 132)
    {
    pos[2].assign(-11.75,21.93);
    pos[3].assign(-11.92,12.8);
    pos[4].assign(-12.42,3.08);
    pos[5].assign(-12.68,-4.94);
    pos[6].assign(6,13.56);
    pos[7].assign(0.08,2.75);
    pos[8].assign(10.9,3.17);
    pos[9].assign(18.85,10.69);
    pos[10].assign(1.27,22.52);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 133)
    {
    pos[2].assign(-4.9,21.76);
    pos[3].assign(-5.66,12.63);
    pos[4].assign(-5.92,3.25);
    pos[5].assign(-6.42,-5.96);
    pos[6].assign(9.46,13.39);
    pos[7].assign(6,2.15);
    pos[8].assign(17.83,3);
    pos[9].assign(28.82,10.44);
    pos[10].assign(15.04,22.61);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 134)
    {
    pos[2].assign(-0.59,21.08);
    pos[3].assign(-0.59,12.04);
    pos[4].assign(-1.01,3);
    pos[5].assign(-0.25,-6.46);
    pos[6].assign(12.85,13.39);
    pos[7].assign(11.66,-0.89);
    pos[8].assign(24.59,4.44);
    pos[9].assign(35.41,12.04);
    pos[10].assign(22.65,23.11);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 135)
    {
    pos[2].assign(-0.59,21.08);
    pos[3].assign(-0.59,12.04);
    pos[4].assign(-1.01,3);
    pos[5].assign(-0.25,-6.46);
    pos[6].assign(18.17,13.65);
    pos[7].assign(15.8,-0.46);
    pos[8].assign(28.39,5.37);
    pos[9].assign(40.48,10.18);
    pos[10].assign(30.59,23.03);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 136)
    {
    pos[2].assign(-49.35,20.58);
    pos[3].assign(-45.21,12.97);
    pos[4].assign(-45.72,4.35);
    pos[5].assign(-41.83,19.31);
    pos[6].assign(-36.34,10.69);
    pos[7].assign(-46.31,-4.1);
    pos[8].assign(-37.01,0.89);
    pos[9].assign(-33.55,16.94);
    pos[10].assign(-40.23,27.17);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 137)
    {
    pos[2].assign(-46.82,20.58);
    pos[3].assign(-45.21,12.97);
    pos[4].assign(-45.72,4.35);
    pos[5].assign(-40.56,17.62);
    pos[6].assign(-36.34,10.69);
    pos[7].assign(-46.31,-4.1);
    pos[8].assign(-37.01,0.89);
    pos[9].assign(-33.55,16.94);
    pos[10].assign(-39.72,27.17);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 138)
    {
    pos[2].assign(-46.82,20.58);
    pos[3].assign(-45.21,12.97);
    pos[4].assign(-45.72,4.35);
    pos[5].assign(-40.56,17.62);
    pos[6].assign(-36.34,10.69);
    pos[7].assign(-46.31,-4.1);
    pos[8].assign(-37.01,0.89);
    pos[9].assign(-33.55,16.94);
    pos[10].assign(-39.72,27.17);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 139)
    {
    pos[2].assign(-44.45,21.25);
    pos[3].assign(-43.77,13.39);
    pos[4].assign(-44.03,4.86);
    pos[5].assign(-38.37,18.46);
    pos[6].assign(-36.34,10.69);
    pos[7].assign(-45.04,-4.1);
    pos[8].assign(-37.01,0.89);
    pos[9].assign(-30.51,17.54);
    pos[10].assign(-34.48,26.75);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 140)
    {
    pos[2].assign(-41.07,21.08);
    pos[3].assign(-41.15,12.8);
    pos[4].assign(-41.58,4.35);
    pos[5].assign(-34.39,18.72);
    pos[6].assign(-32.54,10.69);
    pos[7].assign(-41.92,-4.69);
    pos[8].assign(-32.87,-0.13);
    pos[9].assign(-26.11,17.37);
    pos[10].assign(-30.25,23.7);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 141)
    {
    pos[2].assign(-38.45,21.08);
    pos[3].assign(-39.04,13.14);
    pos[4].assign(-39.04,4.18);
    pos[5].assign(-30.42,18.97);
    pos[6].assign(-28.39,10.27);
    pos[7].assign(-38.79,-4.94);
    pos[8].assign(-28.9,0.97);
    pos[9].assign(-21.13,16.94);
    pos[10].assign(-25.86,23.37);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 142)
    {
    pos[2].assign(-34.23,21.42);
    pos[3].assign(-34.56,13.39);
    pos[4].assign(-34.99,3.85);
    pos[5].assign(-26.62,18.3);
    pos[6].assign(-22.14,10.69);
    pos[7].assign(-34.31,-4.01);
    pos[8].assign(-24.76,1.23);
    pos[9].assign(-14.7,16.44);
    pos[10].assign(-20.2,24.04);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 143)
    {
    pos[2].assign(-32.7,21.59);
    pos[3].assign(-32.96,13.31);
    pos[4].assign(-33.04,3.68);
    pos[5].assign(-24.17,18.72);
    pos[6].assign(-20.11,10.86);
    pos[7].assign(-32.28,-3.59);
    pos[8].assign(-24.76,1.23);
    pos[9].assign(-12.68,16.01);
    pos[10].assign(-18,23.79);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 144)
    {
    pos[2].assign(-29.75,21.68);
    pos[3].assign(-30.42,13.39);
    pos[4].assign(-30.42,3.59);
    pos[5].assign(-21.55,18.89);
    pos[6].assign(-16.82,11.28);
    pos[7].assign(-29.66,-4.94);
    pos[8].assign(-22.39,4.69);
    pos[9].assign(-8.11,16.61);
    pos[10].assign(-14.2,23.79);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 145)
    {
    pos[2].assign(-27.13,22.35);
    pos[3].assign(-28.14,13.56);
    pos[4].assign(-28.31,3.51);
    pos[5].assign(-18.08,18.8);
    pos[6].assign(-13.44,11.11);
    pos[7].assign(-27.46,-4.86);
    pos[8].assign(-19.61,4.27);
    pos[9].assign(-2.96,16.18);
    pos[10].assign(-10.14,23.62);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 146)
    {
    pos[2].assign(-24.08,22.18);
    pos[3].assign(-24.34,13.14);
    pos[4].assign(-24.93,3.85);
    pos[5].assign(-24.68,-4.44);
    pos[6].assign(-9.89,17.54);
    pos[7].assign(-13.44,4.01);
    pos[8].assign(-3.55,7.82);
    pos[9].assign(4.9,16.35);
    pos[10].assign(-1.18,24.63);
    pos[11].assign(7.41,-28.44);
    }
    else if(point == 147)
    {
    pos[2].assign(-18.17,22.01);
    pos[3].assign(-18.68,13.14);
    pos[4].assign(-18.42,4.5);
    pos[5].assign(-17.83,-5.45);
    pos[6].assign(-2.7,17.79);
    pos[7].assign(-6,3.25);
    pos[8].assign(7.01,8.92);
    pos[9].assign(18.85,16.1);
    pos[10].assign(7.18,24.8);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 148)
    {
    pos[2].assign(-12.17,21.42);
    pos[3].assign(-12.34,12.89);
    pos[4].assign(-12.08,4.35);
    pos[5].assign(-12.17,-5.2);
    pos[6].assign(3.21,17.62);
    pos[7].assign(3.04,1.9);
    pos[8].assign(15.55,8.75);
    pos[9].assign(24.85,15.42);
    pos[10].assign(15.63,22.86);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 149)
    {
    pos[2].assign(-5.24,21.42);
    pos[3].assign(-5.66,12.72);
    pos[4].assign(-6.34,3.76);
    pos[5].assign(-6.42,-5.37);
    pos[6].assign(10.73,17.7);
    pos[7].assign(10.9,1.65);
    pos[8].assign(24.17,10.61);
    pos[9].assign(34.9,15.25);
    pos[10].assign(23.92,22.94);
    pos[11].assign(47.41,-28.44);
    }
    else if(point == 150)
    {
    pos[2].assign(-0.59,21.08);
    pos[3].assign(-0.59,12.04);
    pos[4].assign(-1.01,3);
    pos[5].assign(-0.25,-6.46);
    pos[6].assign(17.24,18.38);
    pos[7].assign(16.23,2.49);
    pos[8].assign(28.48,10.01);
    pos[9].assign(40.73,15.17);
    pos[10].assign(30.59,23.03);
    pos[11].assign(47.41,-28.44);
    }

    for(int i=1;i<=9;++i)
    {
        pos2[i]=pos[i+1]+e;
    }
    for(int i=0;i<teammate1.size();++i)
    {
        for(int j=0;j<teammate1.size();++j)
        {
            teammates[i][j]=teammate1[j];
        }
    }

    for(int m=0;m<teammate1.size();++m)
    {
        for(int i=0;i<teammate1.size()-1;++i)
        {
            for(int j=i+1;j<teammate1.size();++j)
            {
                if(teammates[m][i]->pos().dist(pos2[m]) > teammates[m][j]->pos().dist(pos2[m]))
                {
                    std::swap(teammates[m][i],teammates[m][j]);
                }
            }
        }
    }
    bool k;
    for(int i=0;i<teammate1.size();++i)
    {
        k=false;
        for(int j=0;j<teammate1.size();++j)
        {
            for(int s=0;s<i;++s)
            {
                if(teammates[i][j]->unum()==tm_end[s]->unum())
                {
                    k=true;
                    break;
                }
            }
            if(k==true)
            {
                k=false;
                continue;
            }
            tm_end[i]=teammates[i][j];
            break;
        }
    }

    
    
    for(int i=0;i<10;++i)
    {
      //  dlog.addText( Logger::TEAM,__FILE__"tm %d =  %d ",i,tm_end[i]->unum() );
        //agent->debugClient().addCircle(pos2[i],1);
    }
    
    for(int i=0;i<teammate1.size();++i)
    {dlog.addText( Logger::TEAM,__FILE__"satre %d ",i );
        for(int j=0;j<teammate1.size();++j)
        {
           // dlog.addText( Logger::TEAM,__FILE__"n = %d ",teammates[i][j]->unum() );
        }
    }  

    for(int i=0;i<teammate1.size();++i)
    {
        if(tm_end[i]->unum()==u)
        {
            return pos2[i];
        }
    }
    return pos[0];
}
Vector2D Bhv_BasicMove::at_position(rcsc::PlayerAgent* agent, int u)
{
    const WorldModel &wm=agent->world();
    int point=areanumber_att(wm);
    Vector2D ball_area=areapos_att(wm);
    Vector2D ball_pos=wm.ball().pos();
    
    Vector2D pos[12];
    Vector2D pos2[10];
    pos[0]=areapos_def(wm);
    Vector2D ball_pos1;
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(self_min<opp_min && self_min < mate_min )
    {
        ball_pos1=wm.ball().inertiaPoint(self_min);
    }
    else if(mate_min<opp_min && mate_min<opp_min )
    {
        ball_pos1=wm.ball().inertiaPoint(mate_min);
    }
    else if(opp_min<self_min && opp_min<mate_min )
    {
        ball_pos1=wm.ball().inertiaPoint(opp_min);
    }
    pos2[0]=ball_pos1;
    Vector2D e=ball_pos1-ball_area;
    Vector2D team[12];
    std::vector <const AbstractPlayerObject *> teammate1;
    std::vector <Vector2D> team1;
    const AbstractPlayerObject *teammate[12];
    const AbstractPlayerObject *teammates[10][12];
    const AbstractPlayerObject *tm_end[10];
	for(int u = 2;u<=11;u++)
    {
		const AbstractPlayerObject * tm = wm.ourPlayer(u);
		if(tm==NULL || tm->unum() < 1)
        {
            continue;
        }
		team[u]=tm->pos();
        teammate[u]=tm;
        team1.push_back(tm->pos());
        teammate1.push_back(tm);
	}


    if(point == 1)
    {
    pos[2].assign(-16.65,1.56);
    pos[3].assign(-6.17,-11.2);
    pos[4].assign(-22.39,-12.38);
    pos[5].assign(-29.58,-29.96);
    pos[6].assign(-45.38,-19.65);
    pos[7].assign(-33.55,-18.89);
    pos[8].assign(-37.1,-8.75);
    pos[9].assign(-15.38,-22.77);
    pos[10].assign(-36.25,4.52);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 2)
    {
    pos[2].assign(-6.68,-19.56);
    pos[3].assign(3.89,-10.18);
    pos[4].assign(-10.73,-7.9);
    pos[5].assign(-16.82,-30.3);
    pos[6].assign(-38.54,-19.9);
    pos[7].assign(-24.25,-19.82);
    pos[8].assign(-37.86,-5.87);
    pos[9].assign(-18.85,8.58);
    pos[10].assign(-24.59,-5.54);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 3)
    {
    pos[2].assign(12.68,-1.99);
    pos[3].assign(18.25,-14.66);
    pos[4].assign(-13.69,7.9);
    pos[5].assign(-1.77,-30.38);
    pos[6].assign(-27.8,-21.59);
    pos[7].assign(-14.45,-19.9);
    pos[8].assign(-27.63,-6.89);
    pos[9].assign(5.24,-19.65);
    pos[10].assign(-6.51,-9.34);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 4)
    {
    pos[2].assign(20.03,7.99);
    pos[3].assign(20.54,-5.28);
    pos[4].assign(8.28,-13.06);
    pos[5].assign(12.25,-27.25);
    pos[6].assign(-19.94,-18.72);
    pos[7].assign(23.92,-16.94);
    pos[8].assign(-20.96,-5.62);
    pos[9].assign(-6.25,-18.63);
    pos[10].assign(-2.54,-3.59);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 5)
    {
    pos[2].assign(26.45,3.51);
    pos[3].assign(30.85,-7.39);
    pos[4].assign(9.55,7.23);
    pos[5].assign(27.55,-26.32);
    pos[6].assign(-8.62,-20.83);
    pos[7].assign(6.51,-20.75);
    pos[8].assign(-6.51,-7.82);
    pos[9].assign(21.63,-18.04);
    pos[10].assign(10.73,-10.44);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 6)
    {
    pos[2].assign(35.75,4.27);
    pos[3].assign(40.06,-7.73);
    pos[4].assign(15.21,9.08);
    pos[5].assign(40.48,-24.8);
    pos[6].assign(-0.08,-19.65);
    pos[7].assign(16.39,-21.34);
    pos[8].assign(0.59,-3.08);
    pos[9].assign(29.92,-18.38);
    pos[10].assign(20.7,-7.06);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 7)
    {
    pos[2].assign(40.82,5.28);
    pos[3].assign(41.66,-7.99);
    pos[4].assign(27.46,-1.56);
    pos[5].assign(47.49,-20.32);
    pos[6].assign(-0.76,-16.69);
    pos[7].assign(27.13,-28.27);
    pos[8].assign(0.76,-0.55);
    pos[9].assign(36.85,-21.68);
    pos[10].assign(25.69,-14.07);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 8)
    {
    pos[2].assign(-20.2,-7.9);
    pos[3].assign(-4.56,-0.21);
    pos[4].assign(-19.77,9.08);
    pos[5].assign(-31.61,-30.04);
    pos[6].assign(-44.03,-8.92);
    pos[7].assign(-32.79,-15.08);
    pos[8].assign(-32.37,0.72);
    pos[9].assign(-5.41,-14.75);
    pos[10].assign(-19.77,-22.01);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 9)
    {
    pos[2].assign(-9.3,-5.28);
    pos[3].assign(7.35,0.38);
    pos[4].assign(-7.94,10.86);
    pos[5].assign(-21.55,-29.03);
    pos[6].assign(-37.35,-12.72);
    pos[7].assign(-20.45,-12.63);
    pos[8].assign(-29.58,0.8);
    pos[9].assign(6.25,-13.23);
    pos[10].assign(-9.97,-22.1);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 10)
    {
    pos[2].assign(0.68,-0.46);
    pos[3].assign(15.72,4.94);
    pos[4].assign(0.59,14.07);
    pos[5].assign(-28.99,-20.58);
    pos[6].assign(-28.82,-3.17);
    pos[7].assign(-14.54,-10.1);
    pos[8].assign(-14.03,4.61);
    pos[9].assign(14.96,-10.61);
    pos[10].assign(0.68,-20.15);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 11)
    {
    pos[2].assign(8.96,-7.48);
    pos[3].assign(24,0.72);
    pos[4].assign(11.66,8.49);
    pos[5].assign(-18.59,-19.82);
    pos[6].assign(-17.24,-1.48);
    pos[7].assign(-6,-9.93);
    pos[8].assign(-5.75,6.46);
    pos[9].assign(22.99,-12.55);
    pos[10].assign(12.93,-22.44);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 12)
    {
    pos[2].assign(18.93,-5.79);
    pos[3].assign(37.27,-1.31);
    pos[4].assign(28.31,6.04);
    pos[5].assign(0.59,-20.92);
    pos[6].assign(-10.14,-10.18);
    pos[7].assign(4.56,-9.85);
    pos[8].assign(-9.89,3.76);
    pos[9].assign(29.75,-14.07);
    pos[10].assign(25.27,-26.58);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 13)
    {
    pos[2].assign(24.25,-10.1);
    pos[3].assign(38.11,-14.49);
    pos[4].assign(37.94,-0.21);
    pos[5].assign(10.73,-18.04);
    pos[6].assign(-7.86,-12.04);
    pos[7].assign(9.21,-4.01);
    pos[8].assign(-8.54,2.07);
    pos[9].assign(35.24,-30.3);
    pos[10].assign(24.59,6.89);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 14)
    {
    pos[2].assign(27.97,-19.48);
    pos[3].assign(38.96,-12.04);
    pos[4].assign(38.7,2.92);
    pos[5].assign(14.7,-14.41);
    pos[6].assign(0.93,-11.79);
    pos[7].assign(22.9,7.56);
    pos[8].assign(0,1.56);
    pos[9].assign(35.24,-30.3);
    pos[10].assign(26.62,-5.37);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 15)
    {
    pos[2].assign(-0.08,-17.54);
    pos[3].assign(-15.55,-25.14);
    pos[4].assign(-19.44,-10.44);
    pos[5].assign(-16.82,6.97);
    pos[6].assign(-31.86,-2.58);
    pos[7].assign(0.59,-1.23);
    pos[8].assign(-45.04,6.97);
    pos[9].assign(-44.7,-24.8);
    pos[10].assign(-32.87,-16.86);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 16)
    {
    pos[2].assign(12,-9.93);
    pos[3].assign(-3.8,-2.92);
    pos[4].assign(-4.06,-18.46);
    pos[5].assign(-22.65,-0.38);
    pos[6].assign(-39.89,-9);
    pos[7].assign(-9.97,6.63);
    pos[8].assign(-40.06,4.52);
    pos[9].assign(-30,-22.77);
    pos[10].assign(-19.01,-14.32);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 17)
    {
    pos[2].assign(13.1,-4.94);
    pos[3].assign(-0.34,-0.13);
    pos[4].assign(-0.08,-14.83);
    pos[5].assign(-25.01,-2.49);
    pos[6].assign(-34.99,-8.41);
    pos[7].assign(-9.97,6.63);
    pos[8].assign(-34.65,4.35);
    pos[9].assign(-25.27,-17.62);
    pos[10].assign(-10.99,-24.21);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 18)
    {
    pos[2].assign(22.82,-6.55);
    pos[3].assign(10.82,-0.89);
    pos[4].assign(11.15,-16.18);
    pos[5].assign(-13.69,-0.13);
    pos[6].assign(-25.61,-7.56);
    pos[7].assign(0.17,8.32);
    pos[8].assign(-26.79,4.61);
    pos[9].assign(-13.27,-16.86);
    pos[10].assign(-0.17,-23.96);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 19)
    {
    pos[2].assign(33.04,-3.85);
    pos[3].assign(20.11,-0.21);
    pos[4].assign(27.21,-16.35);
    pos[5].assign(3.46,-2.83);
    pos[6].assign(-10.23,-8.41);
    pos[7].assign(13.69,12.13);
    pos[8].assign(-11.15,2.49);
    pos[9].assign(2.2,-16.61);
    pos[10].assign(13.69,-22.18);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 20)
    {
    pos[2].assign(37.86,-2.49);
    pos[3].assign(27.55,1.82);
    pos[4].assign(36.17,-20.07);
    pos[5].assign(15.46,-2.49);
    pos[6].assign(0.93,-11.79);
    pos[7].assign(36.17,11.28);
    pos[8].assign(0,1.56);
    pos[9].assign(15.63,-15.68);
    pos[10].assign(24.85,-23.7);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 21)
    {
    pos[2].assign(32.7,-15.76);
    pos[3].assign(31.44,-4.44);
    pos[4].assign(43.35,1.39);
    pos[5].assign(18.08,-9.59);
    pos[6].assign(0.93,-11.79);
    pos[7].assign(36.34,13.73);
    pos[8].assign(0,1.56);
    pos[9].assign(42.76,-20.75);
    pos[10].assign(19.94,3.51);
    pos[11].assign(-46.99,27.25);
    }
    else if(point == 22)
    {
    pos[2].assign(-0.42,-10.44);
    pos[3].assign(-15.38,-1.14);
    pos[4].assign(-1.01,8.75);
    pos[5].assign(-32.2,7.9);
    pos[6].assign(-32.11,-7.06);
    pos[7].assign(-45.04,-15.93);
    pos[8].assign(-45.46,14.24);
    pos[9].assign(-23.15,17.54);
    pos[10].assign(-23.15,-17.03);
    pos[11].assign(54.5,35.11);
    }
    else if(point == 23)
    {
    pos[2].assign(0.68,-10.94);
    pos[3].assign(-14.87,0.04);
    pos[4].assign(-1.01,8.75);
    pos[5].assign(-26.54,11.54);
    pos[6].assign(-26.37,-10.44);
    pos[7].assign(-40.56,-6.46);
    pos[8].assign(-41.24,6.3);
    pos[9].assign(-15.97,20.41);
    pos[10].assign(-16.31,-19.73);
    pos[11].assign(54.5,35.11);
    }
    else if(point == 24)
    {
    pos[2].assign(12.42,-13.99);
    pos[3].assign(0,-0.13);
    pos[4].assign(12.34,14.24);
    pos[5].assign(-14.87,17.54);
    pos[6].assign(-14.79,-17.96);
    pos[7].assign(-30.34,-6.04);
    pos[8].assign(-30.68,6.55);
    pos[9].assign(-3.55,11.45);
    pos[10].assign(-4.39,-12.13);
    pos[11].assign(54.5,35.11);
    }
    else if(point == 25)
    {
    pos[2].assign(15.3,-17.03);
    pos[3].assign(12.25,-0.38);
    pos[4].assign(15.3,18.38);
    pos[5].assign(-0.42,15.93);
    pos[6].assign(-0.34,-17.87);
    pos[7].assign(-24.93,-6.72);
    pos[8].assign(-24.85,5.37);
    pos[9].assign(-13.1,10.61);
    pos[10].assign(-12,-12.46);
    pos[11].assign(54.5,35.11);
    }
    else if(point == 26)
    {
    pos[2].assign(14.45,-10.94);
    pos[3].assign(26.45,0.13);
    pos[4].assign(14.96,11.54);
    pos[5].assign(-0.34,4.94);
    pos[6].assign(0.93,-5.96);
    pos[7].assign(-16.39,-6.3);
    pos[8].assign(-17.24,3.68);
    pos[9].assign(32.7,16.86);
    pos[10].assign(31.86,-17.03);
    pos[11].assign(54.5,35.11);
    }
    else if(point == 27)
    {
    pos[2].assign(26.79,-10.69);
    pos[3].assign(41.66,-6.8);
    pos[4].assign(36.34,-19.9);
    pos[5].assign(42.08,5.79);
    pos[6].assign(-6,4.27);
    pos[7].assign(-6.59,-6.38);
    pos[8].assign(15.46,-5.37);
    pos[9].assign(27.38,11.96);
    pos[10].assign(14.96,4.52);
    pos[11].assign(54.5,35.11);
    }
    else if(point == 28)
    {
    pos[2].assign(25.1,-12.46);
    pos[3].assign(40.65,-6.97);
    pos[4].assign(36.34,-19.9);
    pos[5].assign(40.23,6.63);
    pos[6].assign(-6,4.27);
    pos[7].assign(-6.59,-6.38);
    pos[8].assign(29.32,-0.38);
    pos[9].assign(28.06,12.38);
    pos[10].assign(15.13,-1.31);
    pos[11].assign(54.5,35.11);
    }
    else if(point == 29)
    {
    pos[2].assign(-0.08,17.54);
    pos[3].assign(-15.55,25.14);
    pos[4].assign(-19.44,10.44);
    pos[5].assign(-16.82,-6.97);
    pos[6].assign(-31.86,2.58);
    pos[7].assign(0.59,1.23);
    pos[8].assign(-45.04,-6.97);
    pos[9].assign(-44.7,24.8);
    pos[10].assign(-32.87,16.86);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 30)
    {
    pos[2].assign(12,9.93);
    pos[3].assign(-3.8,2.92);
    pos[4].assign(-4.06,18.46);
    pos[5].assign(-22.65,0.38);
    pos[6].assign(-39.89,9);
    pos[7].assign(-9.97,-6.63);
    pos[8].assign(-40.06,-4.52);
    pos[9].assign(-30,22.77);
    pos[10].assign(-19.01,14.32);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 31)
    {
    pos[2].assign(13.1,4.94);
    pos[3].assign(-0.34,0.13);
    pos[4].assign(-0.08,14.83);
    pos[5].assign(-25.01,2.49);
    pos[6].assign(-34.99,8.41);
    pos[7].assign(-9.97,-6.63);
    pos[8].assign(-34.65,-4.35);
    pos[9].assign(-25.27,17.62);
    pos[10].assign(-10.99,24.21);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 32)
    {
    pos[2].assign(22.82,6.55);
    pos[3].assign(10.82,0.89);
    pos[4].assign(11.15,16.18);
    pos[5].assign(-13.69,0.13);
    pos[6].assign(-25.61,7.56);
    pos[7].assign(0.17,-8.32);
    pos[8].assign(-26.79,-4.61);
    pos[9].assign(-13.27,16.86);
    pos[10].assign(-0.17,23.96);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 33)
    {
    pos[2].assign(33.04,3.85);
    pos[3].assign(20.11,0.21);
    pos[4].assign(27.21,16.35);
    pos[5].assign(3.46,2.83);
    pos[6].assign(-10.23,8.41);
    pos[7].assign(13.69,-12.13);
    pos[8].assign(-11.15,-2.49);
    pos[9].assign(2.2,16.61);
    pos[10].assign(13.69,22.18);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 34)
    {
    pos[2].assign(37.86,2.49);
    pos[3].assign(27.55,-1.82);
    pos[4].assign(36.17,20.07);
    pos[5].assign(15.46,2.49);
    pos[6].assign(0.93,11.79);
    pos[7].assign(36.17,-11.28);
    pos[8].assign(0,-1.56);
    pos[9].assign(15.63,15.68);
    pos[10].assign(24.85,23.7);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 35)
    {
    pos[2].assign(32.7,15.76);
    pos[3].assign(31.44,4.44);
    pos[4].assign(43.35,-1.39);
    pos[5].assign(18.08,9.59);
    pos[6].assign(0.93,11.79);
    pos[7].assign(36.34,-13.73);
    pos[8].assign(0,-1.56);
    pos[9].assign(42.76,20.75);
    pos[10].assign(19.94,-3.51);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 36)
    {
    pos[2].assign(-20.2,7.9);
    pos[3].assign(-4.56,0.21);
    pos[4].assign(-19.77,-9.08);
    pos[5].assign(-31.61,30.04);
    pos[6].assign(-44.03,8.92);
    pos[7].assign(-32.79,15.08);
    pos[8].assign(-32.37,0.72);
    pos[9].assign(-5.41,14.75);
    pos[10].assign(-19.77,22.01);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 37)
    {
    pos[2].assign(-9.3,5.28);
    pos[3].assign(7.35,0.38);
    pos[4].assign(-7.94,-10.86);
    pos[5].assign(-21.55,29.03);
    pos[6].assign(-37.35,12.72);
    pos[7].assign(-20.45,12.63);
    pos[8].assign(-29.58,0.8);
    pos[9].assign(6.25,13.23);
    pos[10].assign(-9.97,22.1);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 38)
    {
    pos[2].assign(0.68,0.46);
    pos[3].assign(15.72,-4.94);
    pos[4].assign(0.59,-14.07);
    pos[5].assign(-28.99,20.58);
    pos[6].assign(-28.82,3.17);
    pos[7].assign(-14.54,10.1);
    pos[8].assign(-14.03,-4.61);
    pos[9].assign(14.96,10.61);
    pos[10].assign(0.68,20.15);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 39)
    {
    pos[2].assign(8.96,7.48);
    pos[3].assign(24,0.72);
    pos[4].assign(11.66,-8.49);
    pos[5].assign(-18.59,19.82);
    pos[6].assign(-17.24,1.48);
    pos[7].assign(-6,9.93);
    pos[8].assign(-5.75,-6.46);
    pos[9].assign(22.99,12.55);
    pos[10].assign(12.93,22.44);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 40)
    {
    pos[2].assign(18.93,5.79);
    pos[3].assign(37.27,1.31);
    pos[4].assign(28.31,-6.04);
    pos[5].assign(0.59,20.92);
    pos[6].assign(-10.14,10.18);
    pos[7].assign(4.56,9.85);
    pos[8].assign(-9.89,-3.76);
    pos[9].assign(29.75,14.07);
    pos[10].assign(25.27,26.58);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 41)
    {
    pos[2].assign(24.25,10.1);
    pos[3].assign(38.11,14.49);
    pos[4].assign(37.94,0.21);
    pos[5].assign(10.73,18.04);
    pos[6].assign(-7.86,12.04);
    pos[7].assign(9.21,4.01);
    pos[8].assign(-8.54,-2.07);
    pos[9].assign(35.24,30.3);
    pos[10].assign(24.59,-6.89);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 42)
    {
    pos[2].assign(27.97,19.48);
    pos[3].assign(38.96,12.04);
    pos[4].assign(38.7,-2.92);
    pos[5].assign(14.7,14.41);
    pos[6].assign(0.93,11.79);
    pos[7].assign(22.9,-7.56);
    pos[8].assign(0,-1.56);
    pos[9].assign(35.24,30.3);
    pos[10].assign(26.62,5.37);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 43)
    {
    pos[2].assign(-16.65,-1.56);
    pos[3].assign(-6.17,11.2);
    pos[4].assign(-22.39,12.38);
    pos[5].assign(-29.58,29.96);
    pos[6].assign(-45.38,19.65);
    pos[7].assign(-33.55,18.89);
    pos[8].assign(-37.1,8.75);
    pos[9].assign(-15.38,22.77);
    pos[10].assign(-36.25,-4.52);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 44)
    {
    pos[2].assign(-6.68,19.56);
    pos[3].assign(3.89,10.18);
    pos[4].assign(-10.73,7.9);
    pos[5].assign(-16.82,30.3);
    pos[6].assign(-38.54,19.9);
    pos[7].assign(-24.25,19.82);
    pos[8].assign(-37.86,5.87);
    pos[9].assign(-18.85,-8.58);
    pos[10].assign(-24.59,5.54);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 45)
    {
    pos[2].assign(12.68,1.99);
    pos[3].assign(18.25,14.66);
    pos[4].assign(-13.69,-7.9);
    pos[5].assign(-1.77,30.38);
    pos[6].assign(-27.8,21.59);
    pos[7].assign(-14.45,19.9);
    pos[8].assign(-27.63,6.89);
    pos[9].assign(5.24,19.65);
    pos[10].assign(-6.51,9.34);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 46)
    {
    pos[2].assign(20.03,-7.99);
    pos[3].assign(20.54,5.28);
    pos[4].assign(8.28,13.06);
    pos[5].assign(12.25,27.25);
    pos[6].assign(-19.94,18.72);
    pos[7].assign(23.92,16.94);
    pos[8].assign(-20.96,5.62);
    pos[9].assign(-6.25,18.63);
    pos[10].assign(-2.54,3.59);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 47)
    {
    pos[2].assign(26.45,-3.51);
    pos[3].assign(30.85,7.39);
    pos[4].assign(9.55,-7.23);
    pos[5].assign(27.55,26.32);
    pos[6].assign(-8.62,20.83);
    pos[7].assign(6.51,20.75);
    pos[8].assign(-6.51,7.82);
    pos[9].assign(21.63,18.04);
    pos[10].assign(10.73,10.44);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 48)
    {
    pos[2].assign(35.75,-4.27);
    pos[3].assign(40.06,7.73);
    pos[4].assign(15.21,-9.08);
    pos[5].assign(40.48,24.8);
    pos[6].assign(-0.08,19.65);
    pos[7].assign(16.39,21.34);
    pos[8].assign(0.59,3.08);
    pos[9].assign(29.92,18.38);
    pos[10].assign(20.7,7.06);
    pos[11].assign(-46.99,-27.25);
    }
    else if(point == 49)
    {
    pos[2].assign(40.82,-5.28);
    pos[3].assign(41.66,7.99);
    pos[4].assign(27.46,1.56);
    pos[5].assign(47.49,20.32);
    pos[6].assign(-0.76,16.69);
    pos[7].assign(27.13,28.27);
    pos[8].assign(0.76,0.55);
    pos[9].assign(36.85,21.68);
    pos[10].assign(25.69,14.07);
    pos[11].assign(-46.99,-27.25);
    }
    
    for(int i=1;i<=9;++i)
    {
        pos2[i]=pos[i+1]+e;
    }
    for(int i=0;i<teammate1.size();++i)
    {
        for(int j=0;j<teammate1.size();++j)
        {
            teammates[i][j]=teammate1[j];
        }
    }

    for(int m=0;m<teammate1.size();++m)
    {
        for(int i=0;i<teammate1.size()-1;++i)
        {
            for(int j=i+1;j<teammate1.size();++j)
            {
                if(teammates[m][i]->pos().dist(pos2[m]) > teammates[m][j]->pos().dist(pos2[m]))
                {
                    std::swap(teammates[m][i],teammates[m][j]);
                }
            }
        }
    }
    bool k;
    for(int i=0;i<teammate1.size();++i)
    {
        k=false;
        for(int j=0;j<teammate1.size();++j)
        {
            for(int s=0;s<i;++s)
            {
                if(teammates[i][j]->unum()==tm_end[s]->unum())
                {
                    k=true;
                    break;
                }
            }
            if(k==true)
            {
                k=false;
                continue;
            }
            tm_end[i]=teammates[i][j];
            break;
        }
    }

    
    
    for(int i=0;i<10;++i)
    {
        //dlog.addText( Logger::TEAM,__FILE__"tm %d =  %d ",i,tm_end[i]->unum() );
        //agent->debugClient().addCircle(pos2[i],0.5);
    }
    
    for(int i=0;i<teammate1.size();++i)
    {//dlog.addText( Logger::TEAM,__FILE__"satre %d ",i );
        for(int j=0;j<teammate1.size();++j)
        {
            //dlog.addText( Logger::TEAM,__FILE__"n = %d ",teammates[i][j]->unum() );
        }
    }  

    for(int i=0;i<teammate1.size();++i)
    {
        if(tm_end[i]->unum()==u)
        {
            if(wm.offsideLineX()<pos2[i].x)
            {
                pos2[i].assign(wm.offsideLineX(),pos2[i].y);
            }
            if(pos2[i].y<-32)
            {
                pos2[i].assign(pos2[i].x,-32);
            }
            if(pos2[i].y>32)
            {
                pos2[i].assign(pos2[i].x,32);
            }
            return pos2[i];
        }
    }
    return pos[0];
}
int Bhv_BasicMove::areanumber_def(const WorldModel & wm)
{
    Vector2D ball_pos;
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(self_min<opp_min && self_min < mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(self_min);
    }
    else if(mate_min<opp_min && mate_min<opp_min )
    {
        ball_pos=wm.ball().inertiaPoint(mate_min);
    }
    else if(opp_min<self_min && opp_min<mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(opp_min);
    }
    Vector2D point[151];
    point[1].assign(-50,-30);
    point[2].assign(-46,-30);
    point[3].assign(-41,-30);
    point[4].assign(-36,-30);
    point[5].assign(-31,-30);
    point[6].assign(-26,-30);
    point[7].assign(-21,-30);
    point[8].assign(-16,-30);
    point[9].assign(-11,-30);
    point[10].assign(-5,-30);
    point[11].assign(5,-30);
    point[12].assign(15,-30);
    point[13].assign(25,-30);
    point[14].assign(35,-30);
    point[15].assign(45,-30);
    point[16].assign(-50,-23);
    point[17].assign(-46,-23);
    point[18].assign(-41,-23);
    point[19].assign(-36,-23);
    point[20].assign(-31,-23);
    point[21].assign(-26,-23);
    point[22].assign(-21,-23);
    point[23].assign(-16,-23);
    point[24].assign(-11,-23);
    point[25].assign(-5,-23);
    point[26].assign(5,-23);
    point[27].assign(15,-23);
    point[28].assign(25,-23);
    point[29].assign(35,-23);
    point[30].assign(45,-23);
    point[31].assign(-50,-16);
    point[32].assign(-46,-16);
    point[33].assign(-41,-16);
    point[34].assign(-36,-16);
    point[35].assign(-31,-16);
    point[36].assign(-26,-16);
    point[37].assign(-21,-16);
    point[38].assign(-16,-16);
    point[39].assign(-11,-16);
    point[40].assign(-5,-16);
    point[41].assign(5,-16);
    point[42].assign(15,-16);
    point[43].assign(25,-16);
    point[44].assign(35,-16);
    point[45].assign(45,-16);
    point[46].assign(-50,-9);
    point[47].assign(-46,-9);
    point[48].assign(-41,-9);
    point[49].assign(-36,-9);
    point[50].assign(-31,-9);
    point[51].assign(-26,-9);
    point[52].assign(-21,-9);
    point[53].assign(-16,-9);
    point[54].assign(-11,-9);
    point[55].assign(-5,-9);
    point[56].assign(5,-9);
    point[57].assign(15,-9);
    point[58].assign(25,-9);
    point[59].assign(35,-9);
    point[60].assign(45,-9);
    point[61].assign(-50,-3);
    point[62].assign(-46,-3);
    point[63].assign(-41,-3);
    point[64].assign(-36,-3);
    point[65].assign(-31,-3);
    point[66].assign(-26,-3);
    point[67].assign(-21,-3);
    point[68].assign(-16,-3);
    point[69].assign(-11,-3);
    point[70].assign(-5,-3);
    point[71].assign(5,-3);
    point[72].assign(15,-3);
    point[73].assign(25,-3);
    point[74].assign(35,-3);
    point[75].assign(45,-3);
    point[76].assign(-50,3);
    point[77].assign(-46,3);
    point[78].assign(-41,3);
    point[79].assign(-36,3);
    point[80].assign(-31,3);
    point[81].assign(-26,3);
    point[82].assign(-21,3);
    point[83].assign(-16,3);
    point[84].assign(-11,3);
    point[85].assign(-5,3);
    point[86].assign(5,3);
    point[87].assign(15,3);
    point[88].assign(25,3);
    point[89].assign(35,3);
    point[90].assign(45,3);
    point[91].assign(-50,9);
    point[92].assign(-46,9);
    point[93].assign(-41,9);
    point[94].assign(-36,9);
    point[95].assign(-31,9);
    point[96].assign(-26,9);
    point[97].assign(-21,9);
    point[98].assign(-16,9);
    point[99].assign(-11,9);
    point[100].assign(-5,9);
    point[101].assign(5,9);
    point[102].assign(15,9);
    point[103].assign(25,9);
    point[104].assign(35,9);
    point[105].assign(45,9);
    point[106].assign(-50,16);
    point[107].assign(-46,16);
    point[108].assign(-41,16);
    point[109].assign(-36,16);
    point[110].assign(-31,16);
    point[111].assign(-26,16);
    point[112].assign(-21,16);
    point[113].assign(-16,16);
    point[114].assign(-11,16);
    point[115].assign(-5,16);
    point[116].assign(5,16);
    point[117].assign(15,16);
    point[118].assign(25,16);
    point[119].assign(35,16);
    point[120].assign(45,16);
    point[121].assign(-50,23);
    point[122].assign(-46,23);
    point[123].assign(-41,23);
    point[124].assign(-36,23);
    point[125].assign(-31,23);
    point[126].assign(-26,23);
    point[127].assign(-21,23);
    point[128].assign(-16,23);
    point[129].assign(-11,23);
    point[130].assign(-5,23);
    point[131].assign(5,23);
    point[132].assign(15,23);
    point[133].assign(25,23);
    point[134].assign(35,23);
    point[135].assign(45,23);
    point[136].assign(-50,30);
    point[137].assign(-46,30);
    point[138].assign(-41,30);
    point[139].assign(-36,30);
    point[140].assign(-31,30);
    point[141].assign(-26,30);
    point[142].assign(-21,30);
    point[143].assign(-16,30);
    point[144].assign(-11,30);
    point[145].assign(-5,30);
    point[146].assign(5,30);
    point[147].assign(15,30);
    point[148].assign(25,30);
    point[149].assign(35,30);
    point[150].assign(45,30);
    
    int number=1;
    double dist_point;
    double nearst_point=point[1].dist(ball_pos);
    for(int i=1;i<=150;i++)
    {
        dist_point=ball_pos.dist(point[i]);
        if(dist_point<nearst_point)
        {
            nearst_point=dist_point;
            number=i;
        }   
    }
  return number;
}

Vector2D Bhv_BasicMove::areapos_def(const WorldModel & wm)
{
    Vector2D ball_pos;
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(self_min<opp_min && self_min < mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(self_min);
    }
    else if(mate_min<opp_min && mate_min<opp_min )
    {
        ball_pos=wm.ball().inertiaPoint(mate_min);
    }
    else if(opp_min<self_min && opp_min<mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(opp_min);
    }
    Vector2D point[151];
    point[1].assign(-50,-30);
    point[2].assign(-46,-30);
    point[3].assign(-41,-30);
    point[4].assign(-36,-30);
    point[5].assign(-31,-30);
    point[6].assign(-26,-30);
    point[7].assign(-21,-30);
    point[8].assign(-16,-30);
    point[9].assign(-11,-30);
    point[10].assign(-5,-30);
    point[11].assign(5,-30);
    point[12].assign(15,-30);
    point[13].assign(25,-30);
    point[14].assign(35,-30);
    point[15].assign(45,-30);
    point[16].assign(-50,-23);
    point[17].assign(-46,-23);
    point[18].assign(-41,-23);
    point[19].assign(-36,-23);
    point[20].assign(-31,-23);
    point[21].assign(-26,-23);
    point[22].assign(-21,-23);
    point[23].assign(-16,-23);
    point[24].assign(-11,-23);
    point[25].assign(-5,-23);
    point[26].assign(5,-23);
    point[27].assign(15,-23);
    point[28].assign(25,-23);
    point[29].assign(35,-23);
    point[30].assign(45,-23);
    point[31].assign(-50,-16);
    point[32].assign(-46,-16);
    point[33].assign(-41,-16);
    point[34].assign(-36,-16);
    point[35].assign(-31,-16);
    point[36].assign(-26,-16);
    point[37].assign(-21,-16);
    point[38].assign(-16,-16);
    point[39].assign(-11,-16);
    point[40].assign(-5,-16);
    point[41].assign(5,-16);
    point[42].assign(15,-16);
    point[43].assign(25,-16);
    point[44].assign(35,-16);
    point[45].assign(45,-16);
    point[46].assign(-50,-9);
    point[47].assign(-46,-9);
    point[48].assign(-41,-9);
    point[49].assign(-36,-9);
    point[50].assign(-31,-9);
    point[51].assign(-26,-9);
    point[52].assign(-21,-9);
    point[53].assign(-16,-9);
    point[54].assign(-11,-9);
    point[55].assign(-5,-9);
    point[56].assign(5,-9);
    point[57].assign(15,-9);
    point[58].assign(25,-9);
    point[59].assign(35,-9);
    point[60].assign(45,-9);
    point[61].assign(-50,-3);
    point[62].assign(-46,-3);
    point[63].assign(-41,-3);
    point[64].assign(-36,-3);
    point[65].assign(-31,-3);
    point[66].assign(-26,-3);
    point[67].assign(-21,-3);
    point[68].assign(-16,-3);
    point[69].assign(-11,-3);
    point[70].assign(-5,-3);
    point[71].assign(5,-3);
    point[72].assign(15,-3);
    point[73].assign(25,-3);
    point[74].assign(35,-3);
    point[75].assign(45,-3);
    point[76].assign(-50,3);
    point[77].assign(-46,3);
    point[78].assign(-41,3);
    point[79].assign(-36,3);
    point[80].assign(-31,3);
    point[81].assign(-26,3);
    point[82].assign(-21,3);
    point[83].assign(-16,3);
    point[84].assign(-11,3);
    point[85].assign(-5,3);
    point[86].assign(5,3);
    point[87].assign(15,3);
    point[88].assign(25,3);
    point[89].assign(35,3);
    point[90].assign(45,3);
    point[91].assign(-50,9);
    point[92].assign(-46,9);
    point[93].assign(-41,9);
    point[94].assign(-36,9);
    point[95].assign(-31,9);
    point[96].assign(-26,9);
    point[97].assign(-21,9);
    point[98].assign(-16,9);
    point[99].assign(-11,9);
    point[100].assign(-5,9);
    point[101].assign(5,9);
    point[102].assign(15,9);
    point[103].assign(25,9);
    point[104].assign(35,9);
    point[105].assign(45,9);
    point[106].assign(-50,16);
    point[107].assign(-46,16);
    point[108].assign(-41,16);
    point[109].assign(-36,16);
    point[110].assign(-31,16);
    point[111].assign(-26,16);
    point[112].assign(-21,16);
    point[113].assign(-16,16);
    point[114].assign(-11,16);
    point[115].assign(-5,16);
    point[116].assign(5,16);
    point[117].assign(15,16);
    point[118].assign(25,16);
    point[119].assign(35,16);
    point[120].assign(45,16);
    point[121].assign(-50,23);
    point[122].assign(-46,23);
    point[123].assign(-41,23);
    point[124].assign(-36,23);
    point[125].assign(-31,23);
    point[126].assign(-26,23);
    point[127].assign(-21,23);
    point[128].assign(-16,23);
    point[129].assign(-11,23);
    point[130].assign(-5,23);
    point[131].assign(5,23);
    point[132].assign(15,23);
    point[133].assign(25,23);
    point[134].assign(35,23);
    point[135].assign(45,23);
    point[136].assign(-50,30);
    point[137].assign(-46,30);
    point[138].assign(-41,30);
    point[139].assign(-36,30);
    point[140].assign(-31,30);
    point[141].assign(-26,30);
    point[142].assign(-21,30);
    point[143].assign(-16,30);
    point[144].assign(-11,30);
    point[145].assign(-5,30);
    point[146].assign(5,30);
    point[147].assign(15,30);
    point[148].assign(25,30);
    point[149].assign(35,30);
    point[150].assign(45,30);
    
    int number=1;
    double dist_point;
    double nearst_point=point[1].dist(ball_pos);
    for(int i=1;i<=150;i++)
    {
        dist_point=ball_pos.dist(point[i]);
        if(dist_point<nearst_point)
        {
            nearst_point=dist_point;
            number=i;
        }   
    }
  return point[number];
}




int Bhv_BasicMove::areanumber_att(const WorldModel & wm)
{
    Vector2D ball_pos;
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(self_min<opp_min && self_min < mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(self_min);
    }
    else if(mate_min<opp_min && mate_min<opp_min )
    {
        ball_pos=wm.ball().inertiaPoint(mate_min);
    }
    else if(opp_min<self_min && opp_min<mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(opp_min);
    }
    Vector2D point[50];
    point[1].assign(-45,-30);
    point[2].assign(-30,-30);
    point[3].assign(-15,-30);
    point[4].assign( 0 ,-30);
    point[5].assign( 15,-30);
    point[6].assign( 30,-30);
    point[7].assign( 45,-30);
    point[8].assign(-45,-20);
    point[9].assign(-30,-20);
    point[10].assign(-15,-20);
    point[11].assign( 0 ,-20);
    point[12].assign( 15,-20);
    point[13].assign( 30,-20);
    point[14].assign( 45,-20);
    point[15].assign(-45,-10);
    point[16].assign(-30,-10);
    point[17].assign(-15,-10);
    point[18].assign( 0 ,-10);
    point[19].assign( 15,-10);
    point[20].assign( 30,-10);
    point[21].assign( 45,-10);
    point[22].assign(-45,0);
    point[23].assign(-30,0);
    point[24].assign(-15,0);
    point[25].assign( 0 ,0);
    point[26].assign( 15,0);
    point[27].assign( 30,0);
    point[28].assign( 45,0);
    point[29].assign(-45,10);
    point[30].assign(-30,10);
    point[31].assign(-15,10);
    point[32].assign( 0 ,10);
    point[33].assign( 15,10);
    point[34].assign( 30,10);
    point[35].assign( 45,10);
    point[36].assign(-45,20);
    point[37].assign(-30,20);
    point[38].assign(-15,20);
    point[39].assign( 0 ,20);
    point[40].assign( 15,20);
    point[41].assign( 30,20);
    point[42].assign( 45,20);
    point[43].assign(-45,30);
    point[44].assign(-30,30);
    point[45].assign(-15,30);
    point[46].assign( 0 ,30);
    point[47].assign( 15,30);
    point[48].assign( 30,30);
    point[49].assign( 45,30);
    
    int number=1;
    double dist_point;
    double nearst_point=point[1].dist(ball_pos);
    for(int i=1;i<=50;i++)
    {
        dist_point=ball_pos.dist(point[i]);
        if(dist_point<nearst_point)
        {
            nearst_point=dist_point;
            number=i;
        }   
    }
  return number;
}


Vector2D Bhv_BasicMove::areapos_att(const WorldModel & wm)
{
    Vector2D ball_pos;
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(self_min<opp_min && self_min < mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(self_min);
    }
    else if(mate_min<opp_min && mate_min<opp_min )
    {
        ball_pos=wm.ball().inertiaPoint(mate_min);
    }
    else if(opp_min<self_min && opp_min<mate_min )
    {
        ball_pos=wm.ball().inertiaPoint(opp_min);
    }
    Vector2D point[50];
    point[1].assign(-45,-30);
    point[2].assign(-30,-30);
    point[3].assign(-15,-30);
    point[4].assign( 0 ,-30);
    point[5].assign( 15,-30);
    point[6].assign( 30,-30);
    point[7].assign( 45,-30);
    point[8].assign(-45,-20);
    point[9].assign(-30,-20);
    point[10].assign(-15,-20);
    point[11].assign( 0 ,-20);
    point[12].assign( 15,-20);
    point[13].assign( 30,-20);
    point[14].assign( 45,-20);
    point[15].assign(-45,-10);
    point[16].assign(-30,-10);
    point[17].assign(-15,-10);
    point[18].assign( 0 ,-10);
    point[19].assign( 15,-10);
    point[20].assign( 30,-10);
    point[21].assign( 45,-10);
    point[22].assign(-45,0);
    point[23].assign(-30,0);
    point[24].assign(-15,0);
    point[25].assign( 0 ,0);
    point[26].assign( 15,0);
    point[27].assign( 30,0);
    point[28].assign( 45,0);
    point[29].assign(-45,10);
    point[30].assign(-30,10);
    point[31].assign(-15,10);
    point[32].assign( 0 ,10);
    point[33].assign( 15,10);
    point[34].assign( 30,10);
    point[35].assign( 45,10);
    point[36].assign(-45,20);
    point[37].assign(-30,20);
    point[38].assign(-15,20);
    point[39].assign( 0 ,20);
    point[40].assign( 15,20);
    point[41].assign( 30,20);
    point[42].assign( 45,20);
    point[43].assign(-45,30);
    point[44].assign(-30,30);
    point[45].assign(-15,30);
    point[46].assign( 0 ,30);
    point[47].assign( 15,30);
    point[48].assign( 30,30);
    point[49].assign( 45,30);
    
    int number=1;
    double dist_point;
    double nearst_point=point[1].dist(ball_pos);
    for(int i=1;i<=50;i++)
    {
        dist_point=ball_pos.dist(point[i]);
        if(dist_point<nearst_point)
        {
            nearst_point=dist_point;
            number=i;
        }   
    }
  return point[number];
}

bool Bhv_BasicMove::mark(rcsc::PlayerAgent* agent)
{
    const WorldModel &wm = agent->world();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    Vector2D ball_pos;
    if(mate_min < opp_min)
    {
        ball_pos=wm.ball().inertiaPoint(mate_min);
    }
    else
    {
        ball_pos=wm.ball().inertiaPoint(mate_min);
    }
    Vector2D self_pos=def_position(agent,wm.self().unum());
    Vector2D vec_self=wm.self().pos();
    double *d,*d2;
    const PlayerObject *near_op= wm.getOpponentNearestTo(self_pos,5,d);
    const PlayerObject *near_op2= wm.getOpponentNearestTo(wm.self().pos(),5,d2);
    if(near_op==NULL || near_op2==NULL)
    {
        return false;
    }
    Vector2D vec_op=near_op->pos();
    Vector2D vec_op2=near_op2->pos();
    AngleDeg ang_op=(ball_pos-vec_op).th();
    AngleDeg ang_op2=(ball_pos-vec_op2).th();
    Vector2D vec_mark=vec_op+Vector2D::polar2vector(0.5,ang_op);
    Vector2D vec_mark2=vec_op2+Vector2D::polar2vector(0.8,ang_op2);
    double dist=wm.getDistOpponentNearestTo(self_pos,5);
    double dist2=wm.getDistOpponentNearestTo(wm.self().pos(),5);
    double dist_tm=wm.getDistTeammateNearestTo(vec_op2,5);
    if(dist2<2 &&vec_self.x<-30 && vec_self.y<20 && vec_self.y>-20 && dist2<dist_tm)
    {
        agent->debugClient().addCircle(vec_mark,3);
        if(!Body_GoToPoint(vec_mark2,0.3,get_normal_dash_power(wm)).execute(agent))
        {
            Body_TurnToBall().execute(agent);
            return true;
        }
        return true;
    }
    if(dist<4)
    {
        agent->debugClient().addCircle(vec_mark,3);
        if(!Body_GoToPoint(vec_mark,0.2,get_normal_dash_power(wm)).execute(agent))
        {
            Body_TurnToBall().execute(agent);
            return true;
        }
        return true;
    }
    return false;
}







