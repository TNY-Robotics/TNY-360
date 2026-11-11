#pragma once
#include "locomotion/providers/MPAnimation.hpp"

namespace Animations
{
    AnimationSequence StandUp = {
        .frames = {
            {1.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
            }, .ear_r_rad = 1.570f, .ear_l_rad = 1.570f } },
            {1.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            {1.5f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
        }
    };
    AnimationSequence SitDown = {
        .frames = {
            {1.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 50), DEG_TO_RAD( 100) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            {1.5f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            {1.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
                { DEG_TO_RAD(  45), DEG_TO_RAD(- 60), DEG_TO_RAD( 150) },
            }, .ear_r_rad = 1.570f, .ear_l_rad = 1.570f } },
        }
    };

    AnimationSequence BackFallRecover = {
        .frames = {
            { 5.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(- 44), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 4.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(- 89), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(   0), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(   0), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(  44), DEG_TO_RAD(   0), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(   0), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 2.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(- 44), DEG_TO_RAD(   0), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(   0), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(  44), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 4.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(- 44), DEG_TO_RAD(   0), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(- 44), DEG_TO_RAD(   0), DEG_TO_RAD(   0) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(   0), DEG_TO_RAD(-134), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
            { 4.0f, BodyJointState{ .leg_joints = {
                { DEG_TO_RAD(   15), DEG_TO_RAD(- 60), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(   15), DEG_TO_RAD(- 60), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(   15), DEG_TO_RAD(- 60), DEG_TO_RAD( 149) },
                { DEG_TO_RAD(   15), DEG_TO_RAD(- 60), DEG_TO_RAD( 149) },
            }, .ear_r_rad = 0.785f, .ear_l_rad = 0.785f } },
        }
    };
}