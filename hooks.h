#pragma once
#include "sdk.h"
typedef void(__stdcall* CreateMoveFn)(int, float, bool);
#define HookAdd(x) hook.push_back(x)
std::vector<bool(*)(CUserCmd*)> hook;
CreateMoveFn oCreateMove;
bool BHOP(CUserCmd* cmd)
{
	CBaseEntity* me = Interfaces.ClientEntList->GetClientEntity(Interfaces.Engine->GetLocalPlayer());
	if (!me)
		return false;

	if (cmd->buttons & IN_JUMP && !(me->GetFlags() & FL_ONGROUND))
		cmd->buttons &= ~IN_JUMP;
	
	return true;
}
bool runonce = false;
void __stdcall hkCreateMove(int sequence_number,
	float input_sample_frametime,
	bool active)
{
	oCreateMove(sequence_number, input_sample_frametime, active);
	CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces.Input + 0xEC);
	CUserCmd* cmd = &cmdlist[sequence_number % 150];
	if (!runonce)
	{
		HookAdd(BHOP);
		//Add hooks here
		runonce = true;
	}
	for (unsigned int i = 0; i < hook.size(); i++)
	{
		if ((*hook[i])(cmd) == false)
		{
			printf("A plugin failed! Index %i\n", i);
		}
	}
	CVerifiedUserCmd* verifiedlist = *(CVerifiedUserCmd**)((DWORD)Interfaces.Input + 0xF0);
	CVerifiedUserCmd* verified = &verifiedlist[sequence_number % 150];
	verified->m_cmd = *cmd;
	verified->m_crc = gCRC.CRC32_ProcessSingleBuffer(cmd, sizeof(cmd));
}
class CHooks
{
public:
	void Hook()
	{
		CVMTHookManager* ClientHook = new CVMTHookManager(Interfaces.Client);
		oCreateMove = (CreateMoveFn)ClientHook->HookFunction(21, hkCreateMove);
		ClientHook->HookTable(true);
	}
};
CHooks Hooks;