#include "pch.h"
#include "GameESP.h"
#include "Misc.h"
#include "aobscan.h"
#include <stdio.h>

DxManager *pDxm = NULL;
D3DMenu* pMenu = NULL;
MemoryManager *pMM = NULL;



//------ȫ��ʵʱ���µĵ�ַ-------
DWORD dwJuzhenAddr = 0;
DWORD dwPeople[1000];
DWORD dwPeopleCount;

DWORD dwGoods[2000];
DWORD dwGoodsCount;

DWORD dwCars[1000];
DWORD dwCarsCount;

DWORD MyTeamId;
DWORD dwLocalPlayerAddr;
//------ȫ��ʵʱ���µĵ�ַ-------




CRITICAL_SECTION cs;//�����ٽ�������
DWORD dwGamePid;


//fcd7????00002000 ����
//98??????08000400 ����
//1676��Ѫ��ƫ�ƣ�
//3440  3780������ƫ�ƣ�

//������
//�����ַ��C8 ?? ?? 2A 00 00 20 00
//�������ݣ�98 ?? ?? 2A 00 00 20 00
//�����ַ��AB AA AA 3E 00 00 00 80 00 00 00 80 00 00 80 3F 00 00 00 80 61 8B 98 3F
//�˻�ƫ�ƣ�һ��18 ���� 10����684
//Ѫ��ƫ�ƣ�һ��18 ����68C
// 6b0
//���ƫ�ƣ�һ��18 ����2CC ����14
//����ƫ�ƣ�һ��18 ����10D0  ����ƫ��650 ����ƫ��574  ����ƫ��610  ����ƫ��628
//˲��ƫ�ƣ�һ��18 ����10D0  ����ƫ��650 ֱ��2EC


//����������FC D7 ? ? ? ? 00 00 20 00
//���ѱ�ţ� + 24 + 716 + 20
//Ѫ���� + 24 + 1676
//�޺� + 24 + 4272 + 1616 + 1732
//���⣺ + 24 + 4272 + 1616 + 1756
//˳���� + 24 + 4272 + 1616 + 748
//���뿪���� + 24 + 4272 + 1616 + 1480
//����״̬�� + 24 + 4432
//���˶�̬��4004
//�ϵ��ӽǣ� + 24 + 2704 + 288
//�ϵ��ӽǣ� + 24 + 3024 + 288
//�˻��жϣ� + 24 + 684
//����������98 ? ? ? ? ? ? 08 00 04 00
//��Ʒ������9C ? ? ? ? ? ? 00 00 20 00
//�ؾ�������EC ? ? ? ? ? ? 00 00 20 00

//9C ?? ?? 2A 00 00 20 00


namespace GameData {
	BYTE bJuzhenTag[] = { 0xAB,0xAA,0xAA,0x3E,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x80,0x61,0x8B,0x98,0x3F };
	BYTE bPeopleTag[] = { 0x10,'?', '?',0x2a,0x00,0x00,0x20,0x00,'?','?','?',0x00,0x00};
	BYTE bLocalPlayerTag[] = { 0x98,'?','?',0x2A ,0x00 ,0x00 ,0x20 ,0x00 };
	//BYTE bGoodsTag[] = { 0x9C,'?','?','?', 0x00 ,0x00 ,0x20 ,0x00 };
	//BYTE bGoodsTag[] = { 0x9C,0xdf,'?','?', 0x00 ,0x00 ,0x20 ,0x00 };
	BYTE bGoodsTag[] = { '?','?','?','?', 0x00 ,0x00 ,0x20 ,0x00,'?','?',0x03,0x00 };
	BYTE bCarsTag[] = { 0x88,'?', '?', '?',0x00,0x00,0x20,0x00 };
	DWORD OFFSET_1 = 0x18; //24
	DWORD OFFSET_2 = 0x120; //288
	DWORD BLOOD_OFFSET = 0x6b0;
	DWORD POS_OFFSET = 0xec4;
	DWORD DUIYOU_OFFSET = 0x2cc;
	DWORD RENJI_OFFSET = 684;
	DWORD STATUS_OFFSET = 0xAE0;

	DWORD B_GOODS_OFFSET = 132;//�ж��Ƿ�Ϊ��Ʒ
	DWORD GOODS_ID_OFFSET = 272;//��Ʒidƫ��
	DWORD GOODS_POS_OFFSET = 0x150;//0x150;//��Ʒ����ƫ��
}


namespace global
{
	int bEsp = false;
	int bPlayer = false;
	int bVehicle = true;
	int bGoods = false;

	int bAim = false;
	int bAimOpen = true;
	int nAimPos = 0;

	int bAbout = 0;
	int nDebug = 0;

	int nOther = 0;
	int bSpeed = false;
	int bWeapon = false;
	int bXiGoods = false;
	int bMiniMap = false;
}

const char *opt_Grp[] = { "+", "-" };
const char *opt_OFFON[] = { "��", "��" };
const char *sRadar[] = { "��", "����", "������" };
const char *sAimSlot[] = { "ͷ��", "����", "�ز�", "�ɲ�" };
const char *sSpeedHax[] = { "��", "1.5��", "2��", "2.5��" ,"5��" };

void ShowMenu()
{
	
	if (pMenu == 0) {
		pMenu = new D3DMenu((char*)"�˵�ѡ��", 390, 134);
		pMenu->dxm = pDxm;
		pMenu->visible = 1;
		pMenu->col_title = MCOLOR_TITLE;
	}
	else
	{
		pMenu->dxm = pDxm;
		if (pMenu->noitems == 0)
		{
			pMenu->AddGroup((char*)"[͸ҕ]", &global::bEsp, (char**)opt_Grp);
			if (global::bEsp)
			{
				pMenu->AddItem((char*)"����͸ҕ", &global::bPlayer, (char**)opt_OFFON);
				pMenu->AddItem((char*)"��Ʒ͸ҕ", &global::bGoods, (char**)opt_OFFON);
				pMenu->AddItem((char*)"����͸ҕ", &global::bVehicle, (char**)opt_OFFON);
			}
			pMenu->AddGroup((char*)"[����]", &global::bAim, (char**)opt_Grp);
			if (global::bAim)
			{
				pMenu->AddItem((char*)"���鿪��", &global::bAimOpen, (char**)opt_OFFON);
				pMenu->AddItem((char*)"����λ��", &global::nAimPos, (char**)sAimSlot, 4);
			}
			//����
			pMenu->AddGroup((char*)"[����]", &global::nOther, (char**)opt_Grp);
			if (global::nOther)
			{
				pMenu->AddItem((char*)"��С��ͼ", &global::bMiniMap, (char**)opt_OFFON);
				pMenu->AddItem((char*)"�޺�����", &global::bWeapon, (char**)opt_OFFON);
				pMenu->AddItem((char*)"������Ͷ", &global::bXiGoods, (char**)opt_OFFON);
				pMenu->AddItem((char*)"ȫ�ּ���", &global::bSpeed, (char**)sSpeedHax, 4);
			}
			pMenu->AddGroup((char*)"[����]", &global::bAbout, (char**)opt_Grp);
			if (global::bAbout)
			{
				pMenu->AddItem((char*)"��ӡ��־", &global::nDebug, (char**)opt_OFFON);
			}
		}
		pMenu->Show();
		pMenu->Nav();
	}
}


BOOL WorldToScreen2(D3DXVECTOR3 _Enemy, D3DXVECTOR3 &_Screen,int &nLen)
{
	_Screen = D3DXVECTOR3(0, 0, 0);
	float ScreenW = 0;
	D3DMATRIX GameViewMatrix = pMM->RPM<D3DMATRIX>(dwJuzhenAddr, sizeof(D3DMATRIX));
	ScreenW = (GameViewMatrix._14 * _Enemy.x) + (GameViewMatrix._24* _Enemy.y) + (GameViewMatrix._34 * _Enemy.z + GameViewMatrix._44);
	nLen = ScreenW / 100;
	if (ScreenW < 0.0001f)
	{ 
		return FALSE;
	}
		

	float ScreenY = (GameViewMatrix._12 * _Enemy.x) + (GameViewMatrix._22 * _Enemy.y) + (GameViewMatrix._32 * (_Enemy.z+85) + GameViewMatrix._42);
	float ScreenX = (GameViewMatrix._11 * _Enemy.x) + (GameViewMatrix._21 * _Enemy.y) + (GameViewMatrix._31 * _Enemy.z + GameViewMatrix._41);

	_Screen.y = (pDxm->s_height / 2) - (pDxm->s_height / 2) * ScreenY / ScreenW;
	_Screen.x = (pDxm->s_width / 2) + (pDxm->s_width / 2) * ScreenX / ScreenW;
	float y1 = (pDxm->s_height / 2) - (GameViewMatrix._12*_Enemy.x + GameViewMatrix._22 * _Enemy.y +GameViewMatrix._32 *(_Enemy.z -95 ) +GameViewMatrix._42) *(pDxm->s_height / 2)/ ScreenW;
	_Screen.z=y1 - _Screen.y;
	return TRUE;
}

BOOL WorldToScreenGoods(D3DXVECTOR3 _Enemy, D3DXVECTOR3 &_Screen, int &nLen)
{
	_Screen = D3DXVECTOR3(0, 0, 0);
	float ScreenW = 0;
	D3DMATRIX GameViewMatrix = pMM->RPM<D3DMATRIX>(dwJuzhenAddr, sizeof(D3DMATRIX));
	ScreenW = (GameViewMatrix._14 * _Enemy.x) + (GameViewMatrix._24* _Enemy.y) + (GameViewMatrix._34 * _Enemy.z + GameViewMatrix._44);
	nLen = ScreenW / 100;
	if (ScreenW < 0.0001f)
	{
		return FALSE;
	}


	float ScreenY = (GameViewMatrix._12 * _Enemy.x) + (GameViewMatrix._22 * _Enemy.y) + (GameViewMatrix._32 * (_Enemy.z + 5) + GameViewMatrix._42);
	float ScreenX = (GameViewMatrix._11 * _Enemy.x) + (GameViewMatrix._21 * _Enemy.y) + (GameViewMatrix._31 * _Enemy.z + GameViewMatrix._41);

	_Screen.y = (pDxm->s_height / 2) - (pDxm->s_height / 2) * ScreenY / ScreenW;
	_Screen.x = (pDxm->s_width / 2) + (pDxm->s_width / 2) * ScreenX / ScreenW;
	float y1 = (pDxm->s_height / 2) - (GameViewMatrix._12*_Enemy.x + GameViewMatrix._22 * _Enemy.y + GameViewMatrix._32 *(_Enemy.z) + GameViewMatrix._42) *(pDxm->s_height / 2) / ScreenW;
	_Screen.z = y1 - _Screen.y;
	return TRUE;
}

DWORD WINAPI ThreadUpdateData(LPVOID p)
{
	while (1)
	{
		
		if (dwJuzhenAddr == 0 || pMM->RPM<float>(dwJuzhenAddr + 56, 4) != 3)
		{
			std::vector<DWORD_PTR> vAddr;
			pMM->MemSearch(GameData::bJuzhenTag, sizeof(GameData::bJuzhenTag), 0x40000000, 0x7fffffff, FALSE, 1, vAddr);
			//vAddr = AobScan::FindSigX32(dwGamePid, "ABAAAA3E00000080000000800000803F00000080618B983F", 0x40000000, 0x7fffffff);
			std::vector<DWORD_PTR>::iterator it;
		
			for (it = vAddr.begin(); it != vAddr.end(); it++)
			{
				DWORD dwTag = pMM->RPM<DWORD>(*it + 0x3b8, sizeof(DWORD));
				//ViewWorld.m[0][3]
				D3DMATRIX dmTmp = pMM->RPM<D3DMATRIX>(*it + 268 - 56, sizeof(D3DMATRIX));
				if (dwTag == 0x80000000)
				{
					dwJuzhenAddr = *it + 268 - 56;
					break;
				}
			}
			printf("juzhen %x \n", dwJuzhenAddr);
		}

		//���¶���
		std::vector<DWORD_PTR> vPeople;
		pMM->MemSearch(GameData::bPeopleTag, sizeof(GameData::bPeopleTag), 0x20327000, 0x7fffffff, FALSE, 0, vPeople);
		//vPeople = AobScan::FindSigX32(dwGamePid, "fcd7????00002000", 0x10000000, 0x7fffffff);
		//LeaveCriticalSection(&cs);//�뿪�ٽ���
 		dwPeopleCount = vPeople.size();
		for (int i=0;i< dwPeopleCount;i++)
		{
			dwPeople[i] = vPeople[i];
		}

		

		Sleep(100);//�������ݵ�ʱ��
	}
	return 1;
}

DWORD WINAPI ThreadUpdateData2(LPVOID p)
{
	while (1)
	{
		//������Ʒ
		std::vector<DWORD_PTR> vGoods;
		pMM->MemSearch(GameData::bGoodsTag, sizeof(GameData::bGoodsTag), 0x20327000, 0x7fffffff, FALSE, 0, vGoods);
		//vGoods = AobScan::FindSigX32(dwGamePid, "9CDF????00002000", 0x10000000, 0x7fffffff);

		dwGoodsCount = vGoods.size();
		for (int i = 0; i < dwGoodsCount; i++)
		{
			dwGoods[i] = vGoods[i];
		}
		//printf("��Ʒ��Ŀ %d\n", dwGoodsCount);
		Sleep(1000 * 2);
	}
}

DWORD WINAPI ThreadUpdateData3(LPVOID p)
{
	while (1)
	{
		//������Ʒ
		std::vector<DWORD_PTR> vCars;
		pMM->MemSearch(GameData::bCarsTag, sizeof(GameData::bCarsTag), 0x00327000, 0x7fffffff, FALSE, 0, vCars);
		//vGoods = AobScan::FindSigX32(dwGamePid, "9CDF????00002000", 0x10000000, 0x7fffffff);

		dwCarsCount = vCars.size();
		for (int i = 0; i < dwCarsCount; i++)
		{
			dwCars[i] = vCars[i];
		}
		//printf("������Ŀ %d\n", dwCarsCount);
		Sleep(1000 * 2);
	}
}

FLOAT GetDistance(FLOAT X, FLOAT Y, FLOAT X1, FLOAT Y1)
{
	FLOAT XX, YY;
	XX = X - X1;
	YY = Y - Y1;
	return sqrt(XX * XX + YY * YY);
}
BOOL bLockAim = FALSE;
float fMin = 0.0f;
float g_AimX = 0.0f;
float g_AimY = 0.0f;


void ESPWork()
{
#pragma region �˵�����
	//ShowMenu();
#pragma endregion 

#pragma region FPS׼�ĵ�����
	pDxm->DrawFPS(pDxm->s_width-320,5,D3DCOLOR_XRGB(168, 39, 136),pDxm->pFont);
	//pDxm->DrawCrosshair();
#pragma endregion 

#pragma region ��Ҫ����

	int nRealPeopleCount = 0;
	fMin = 0.0f;
	g_AimX = 0.0f;
	g_AimY = 0.0f;

	for (int i = 0;i<dwPeopleCount;i++)
	{
		DWORD_PTR dw1 = pMM->RPM<DWORD_PTR>(dwPeople[i] + GameData::OFFSET_1, sizeof(DWORD_PTR));
		DWORD_PTR dw2 = pMM->RPM<DWORD_PTR>(dw1 + GameData::OFFSET_2, sizeof(DWORD_PTR));
		float fBlood = pMM->RPM<float>(dw1 + GameData::BLOOD_OFFSET, sizeof(float));
		
		if (fabs(fBlood) > 1e-5 && fBlood >=0.0 && fBlood<=100)//����0
		{

			//�ж϶���
			DWORD dwTeamId = pMM->RPM<DWORD>(pMM->RPM<DWORD>(dw1 + GameData::DUIYOU_OFFSET, sizeof(DWORD)) + 0x14, sizeof(DWORD));
			//DWORD dwTeamId2 = pMM->RPM<DWORD>(pMM->RPM<DWORD>(dw1 + GameData::DUIYOU_OFFSET+4, sizeof(DWORD)) + 0x14, sizeof(DWORD));
			//printf("����id %d\n", dwTeamId);
			if(dwTeamId<5&& dwTeamId>0)
			{
				continue;
			}
			DWORD tmpid = pMM->RPM<DWORD>(dw1 + 0x14, sizeof(DWORD));
			//printf("����id%d\n",tmpid);
			//if(tmpid ==1)
			//{
			//	continue;
			//}

			nRealPeopleCount++;
			D3DXVECTOR3 vPos = pMM->RPM<D3DXVECTOR3>(dw1 + GameData::POS_OFFSET, sizeof(D3DXVECTOR3));
			D3DXVECTOR3 vret;
			int nLen = 0;

			if (WorldToScreen2(vPos, vret, nLen))
			{	
								
				DWORD dwRenji = pMM->RPM<DWORD>(dw1 + GameData::RENJI_OFFSET, sizeof(DWORD));

				DWORD dwStatus = pMM->RPM<DWORD>(dw1 + GameData::STATUS_OFFSET, sizeof(DWORD));
				if (global::nDebug)
				{
					if (nLen-1==1)
					{
						printf("%d %x %x\n", dwStatus,dwTeamId,dwPeople[i]);
					}
					
				}
				float x = vret.x;
				float y = vret.y;
				float h = vret.z;
				float w = vret.z / 2;

				float aimx = vret.x;
				float aimy = vret.y + vret.z / 4;
				

				if (dwStatus == 1 || dwStatus == 4 )
				{
					y = vret.y+ vret.z / 5;
					h = h - vret.z / 5;
					aimy = aimy + vret.z / 6;
				}
				if (dwStatus == 2 || dwStatus == 5 || dwStatus == 7)
				{
					y = vret.y + vret.z / 4;
					h = h - vret.z / 4;
					aimx = aimx + vret.z / 10;
					aimy = aimy + vret.z / 4;
				}
				if (global::bAimOpen)
				{
					//if (bLockAim == FALSE)//û������ʱ���ټ�����С��Ļ����
					if(1)
					{
						FLOAT CurrentDistance = GetDistance(pDxm->s_width / 2.0, pDxm->s_height / 2.0, aimx, aimy);
						if (fMin == 0)
						{
							fMin = CurrentDistance;
							g_AimX = aimx;
							g_AimY = aimy;
						}
						else
						{
							if (CurrentDistance < fMin)
							{
								fMin = CurrentDistance;
								g_AimX = aimx;
								g_AimY = aimy;
							}
						}
					}
				}
				
				//pDxm->DrawPoint(aimx, aimy, 10, 10, TextBlack);

				if (dwRenji == 0)//���˻�
				{
					pDxm->DrawString(x , y, TextBlue, pDxm->pFont, "[%dm]", nLen - 1);
				}
				else
				{
					pDxm->DrawString(x, y, TextRed, pDxm->pFont, "[%dm]", nLen - 1);
				}
				pDxm->DrawBox(x - vret.z / 4, y, w, h, TextWhite);
				pDxm->DrawBlood(x - vret.z / 4 - 5, y, h, w, fBlood);				
			}
		}
	}
	if(global::bAimOpen)
	{
		bLockAim = TRUE;
		float boneX = g_AimX - pDxm->s_width / 2.0;
		float boneY = g_AimY - pDxm->s_height / 2.0;
		float moveOffsetX = boneX / 5;
		float moveOffsetY = boneY / 4;

		//�������
		if((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0&& boneX >= -80.0 && boneX <= 80.0 && boneY >= -80.0 && boneY <= 80.0)
		{
			mouse_event(MOUSEEVENTF_MOVE, moveOffsetX, moveOffsetY, NULL, NULL);
		}
		//����û����������°���q������e����
		else if (boneX >= -120.0 && boneX <= 120 && boneY >= -120.0 && boneY <= 120.0 
			&& ((GetAsyncKeyState('Q') & 0x8000) != 0|| (GetAsyncKeyState('E') & 0x8000) != 0)
			)
		{
			mouse_event(MOUSEEVENTF_MOVE, moveOffsetX, moveOffsetY, NULL, NULL);
			Sleep(20);
			mouse_event(MOUSEEVENTF_LEFTDOWN, moveOffsetX, moveOffsetY, NULL, NULL);
			Sleep(10);
			mouse_event(MOUSEEVENTF_LEFTUP, moveOffsetX, moveOffsetY, NULL, NULL);
		}
	}
	else
	{
		bLockAim = FALSE;
		g_AimX = 0;
		g_AimY = 0;
	}

	pDxm->DrawString(pDxm->s_width/2-300, pDxm->s_height/11, DARKORANGE, pDxm->pFont, "��������[%d]����", nRealPeopleCount);
#pragma endregion

#pragma region ��Ʒ
	if (global::bGoods)
	{
		int nNowGoodCount = 0;
		for (size_t i = 0; i < dwGoodsCount; i++)
		{
			DWORD_PTR t1 = pMM->RPM<DWORD_PTR>(dwGoods[i] + GameData::B_GOODS_OFFSET, sizeof(DWORD_PTR));
			//if (t1 == 0xa8a0613)
			if(1)
			{
				nNowGoodCount++;
				int nId = pMM->RPM<int>(dwGoods[i] + GameData::GOODS_ID_OFFSET, sizeof(int));
				char szGoodName[] = "xxxx";
				//char szGoodName[20] = { 0 };
				// switch (nId)
				// {
				// case 1098798372:
				// 	strcat(szGoodName, "������");
				// 	break;
				// case 1078228938:
				// 	strcat(szGoodName, "SCAR-L");
				// 	break;
				// case 1070342297:
				// 	strcat(szGoodName, "M416");
				// 	break;
				// case 1075583328:
				// 	strcat(szGoodName, "QBZ");
				// 	break;
				// case 1075923558:
				// 	strcat(szGoodName, "SKS");
				// 	break;
				// case 1081607025:
				// 	strcat(szGoodName, "SLR");
				// 	break;
				// case 1084841537:
				// 	strcat(szGoodName, "������");
				// 	break;
				// case 1103479324:
				// 	strcat(szGoodName, "����ͷ");
				// 	break;
				// case 1089312141:
				// 	strcat(szGoodName, "�˱���");
				// 	break;
				// case 1090071692:
				// 	strcat(szGoodName, "������");
				// 	break;
				// case 1091232615:
				// 	strcat(szGoodName, "�ı���");
				// 	break;
				// case 1080193518:
				// 	strcat(szGoodName, "7.62mm");
				// 	break;
				// case 1079905560:
				// 	strcat(szGoodName, "5.56mm");
				// 	break;
				// default:
				// 	break;
				// }
				// if (szGoodName[0] == 0)
				// {
				// 	break;
				// }
				// printf("%s\n",szGoodName);

				D3DXVECTOR3 vPosGoods = pMM->RPM<D3DXVECTOR3>(dwGoods[i] + GameData::GOODS_POS_OFFSET, sizeof(D3DXVECTOR3));
				D3DXVECTOR3 vret;
				int nLen = 0;

				if (WorldToScreenGoods(vPosGoods, vret, nLen))
				{
					printf("draw %f %f %d\n",vret.x,vret.y,nLen);
					pDxm->DrawString(vret.x, vret.y, TextRed, pDxm->pFont, "%s[%dm]", szGoodName, nLen);
				}
			}
		}

	}
	
#pragma endregion
#pragma region ����
	DWORD current_offset = GameData::GOODS_POS_OFFSET;
	bool willadd = true;
	if (global::bVehicle)
	{
		for (size_t i = 0; i < dwCarsCount; i++)
		{
			D3DXVECTOR3 vPosCars = pMM->RPM<D3DXVECTOR3>(dwCars[i] + current_offset, sizeof(D3DXVECTOR3));
			D3DXVECTOR3 vret;
			int nLen = 0;
			if (WorldToScreenGoods(vPosCars, vret, nLen))
			{
				if (nLen>0 && nLen<1000)
				{
					willadd = false;
					//printf("ok len :%x", current_offset);
					pDxm->DrawString(vret.x, vret.y, TextCyan, pDxm->pFont, "����[%dm]", nLen);
				}
				
			}
		}
	}
	
#pragma endregion


}