#pragma once// #include "testhack.h"#include "Interfaces.h"#include "Menu.h"class CGameFunctions{public:	float random_float(float min, float max)	{		typedef float(*RandomFloat_t)(float, float);		static RandomFloat_t m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle(("vstdlib.dll")), ("RandomFloat"));		return m_RandomFloat(min, max);	}	bool is_close(float value1, float value2, float max_difference)	{		float buffered[2] = { value1, value2 };		if (buffered[0] < 180) buffered[0] += 360;		if (buffered[0] > 180) buffered[0] -= 360;		if (buffered[1] < 180) buffered[1] += 360;		if (buffered[1] > 180) buffered[1] -= 360;		if ((buffered[0] - buffered[1]) >= -max_difference && (buffered[0] - buffered[1]) <= max_difference)			return true;		return false;	}		void calculate_angle(Vector src, Vector dst, Vector &angles)	{		Vector delta = src - dst;		double hyp = delta.Length2D();		angles.y = (atan(delta.y / delta.x) * 57.295779513082f);		angles.x = (atan(delta.z / hyp) * 57.295779513082f);		angles[2] = 0.0f;		if (delta.x >= 0.0) angles.y += 180.0f;	}	FORCEINLINE void vector_subtract(const Vector& a, const Vector& b, Vector& c)	{		c.x = a.x - b.x;		c.y = a.y - b.y;		c.z = a.z - b.z;	}	void inline sincos(float radians, float *sine, float *cosine)	{		*sine = sin(radians);		*cosine = cos(radians);	}	void angle_vectors(const Vector &angles, Vector *forward) {		float sp, sy, cp, cy;		sincos(DEG2RAD(angles[1]), &sy, &cy);		sincos(DEG2RAD(angles[0]), &sp, &cp);		forward->x = cp * cy;		forward->y = cp * sy;		forward->z = -sp;	}	void angle_vectors(const Vector &angles, Vector& forward)	{		Assert(s_bMathlibInitialized);		Assert(forward);		float	sp, sy, cp, cy;		sy = sin(DEG2RAD(angles[1]));		cy = cos(DEG2RAD(angles[1]));		sp = sin(DEG2RAD(angles[0]));		cp = cos(DEG2RAD(angles[0]));		forward.x = cp * cy;		forward.y = cp * sy;		forward.z = -sp;	}	void normalize(Vector &vIn, Vector &vOut)	{		float flLen = vIn.Length();		if (flLen == 0) {			vOut.Init(0, 0, 1);			return;		}		flLen = 1 / flLen;		vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);	}	void angle_vectors(const Vector &angles, Vector *forward, Vector *right, Vector *up)	{		float sr, sp, sy, cr, cp, cy;		sincos(DEG2RAD(angles[1]), &sy, &cy);		sincos(DEG2RAD(angles[0]), &sp, &cp);		sincos(DEG2RAD(angles[2]), &sr, &cr);		if (forward)		{			forward->x = cp * cy;			forward->y = cp * sy;			forward->z = -sp;		}		if (right)		{			right->x = (-1 * sr*sp*cy + -1 * cr*-sy);			right->y = (-1 * sr*sp*sy + -1 * cr*cy);			right->z = -1 * sr*cp;		}		if (up)		{			up->x = (cr*sp*cy + -sr * -sy);			up->y = (cr*sp*sy + -sr * cy);			up->z = cr * cp;		}	}	void vector_transform(const Vector in1, float in2[3][4], Vector &out)	{		out[0] = DotProduct(in1, Vector(in2[0][0], in2[0][1], in2[0][2])) + in2[0][3];		out[1] = DotProduct(in1, Vector(in2[1][0], in2[1][1], in2[1][2])) + in2[1][3];		out[2] = DotProduct(in1, Vector(in2[2][0], in2[2][1], in2[2][2])) + in2[2][3];	}	Vector get_hitbox_location(IClientEntity* obj, int hitbox_id) {		matrix3x4 bone_matrix[128];		if (obj->SetupBones(bone_matrix, 128, 0x00000100, 0.0f)) {			if (obj->GetModel()) {				auto studio_model = interfaces::model_info->GetStudiomodel(obj->GetModel());				if (studio_model) {					auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);					if (hitbox) {						auto min = Vector{}, max = Vector{};						vector_transform(hitbox->bbmin, bone_matrix[hitbox->bone], min);						vector_transform(hitbox->bbmax, bone_matrix[hitbox->bone], max);						return (min + max) / 2.0f;					}				}			}		}		return Vector{};	}	std::string get_time()	{		time_t current_time;		struct tm *time_info;		static char timeString[10];		time(&current_time);		time_info = localtime(&current_time);		strftime(timeString, sizeof(timeString), ("%I:%M%p"), time_info);		return timeString;	}	bool world_to_screen(const Vector &origin, Vector &screen)	{		if (!screen_transform(origin, screen)) {			int iScreenWidth, iScreenHeight;			interfaces::engine->GetScreenSize(iScreenWidth, iScreenHeight);			screen.x = (iScreenWidth / 2.0f) + (screen.x * iScreenWidth) / 2;			screen.y = (iScreenHeight / 2.0f) - (screen.y * iScreenHeight) / 2;			return true;		}		return false;	}	bool screen_transform(const Vector& point, Vector& screen)	{		const matrix3x4& w2sMatrix = interfaces::engine->WorldToScreenMatrix();		screen.x = w2sMatrix[0][0] * point.x + w2sMatrix[0][1] * point.y + w2sMatrix[0][2] * point.z + w2sMatrix[0][3];		screen.y = w2sMatrix[1][0] * point.x + w2sMatrix[1][1] * point.y + w2sMatrix[1][2] * point.z + w2sMatrix[1][3];		screen.z = 0.0f;		float w = w2sMatrix[3][0] * point.x + w2sMatrix[3][1] * point.y + w2sMatrix[3][2] * point.z + w2sMatrix[3][3];		if (w < 0.001f) {			screen.x *= 100000;			screen.y *= 100000;			return true;		}		float invw = 1.0f / w;		screen.x *= invw;		screen.y *= invw;		return false;	}	float get_fraction_props_only(Vector start, Vector end)	{		Ray_t ray;		CGameTrace tr;		CTraceFilter filter;		ray.Init(start, end);		interfaces::trace->TraceRay(ray, MASK_SOLID, &filter, &tr);		return tr.fraction;	}};