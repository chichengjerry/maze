#include "star.h"
#include "game.h"
#include "maze.h"
#include "player.h"

LPDIRECT3DTEXTURE9 STAR::pTex = NULL;
UINT STAR::cnt = 0;

STAR::STAR(MAZE * maze, PLAYER* owner)
{
	pMaze = maze;
	player = owner;
	visible = FALSE;
	throttle = 0.0f;

	if (!cnt)
		D3D::LoadTexture(&pTex, _T("data/TEXTURE/glow01.jpg"));
	
	D3DXVECTOR3 def = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	emitter = new EMITTER(pTex, GM_MAX_PARTICLES, def, def, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 20.0f);
	for (int i = 0; i < emitter->nParticlesCount; i++)
		emitter->particles[i].bAlive = FALSE;

	mdl = NULL;
	cnt++;
}

STAR::~STAR()
{
	SAFE_DELETE(mdl);
	SAFE_DELETE(emitter);
	if (!--cnt)
		SAFE_RELEASE(pTex);
}

HRESULT STAR::Draw()
{
	if (visible) {
		if(emitter) emitter->Draw(player->camera);
		if(mdl) mdl->Draw(NULL);
	}
	return D3D_OK;
}

void STAR::Update()
{
	vector<D3DXVECTOR3>* path = &player->guideVtx;
	throttle += D3D::fAnimationRate;

	const float THROTTLE_LIMIT = 2.0f;

	if (!path->empty()) {
		int s = path->size();
		visible = TRUE;
		int ran = GET_RANDOM(1, 10);

		for (int i = 0; i < GM_MAX_PARTICLES; i++) {
			bool unlit = false;
			if (i < s) {
				emitter->particles[i].pos = path->at(i);
				emitter->particles[i].pos.y = GR_FAR_Z * 0.25f;
				if (player->pGame->gameLevel == LEVEL_HARD) {
					emitter->particles[i].pos += player->srt.pos;
				}

				if ((i == 0 || i == path->size() - 1)) {
					if (!emitter->particles[i].bAlive) {
						emitter->particles[i].bAlive = TRUE;
						emitter->particles[i].scl = 3.0f;
					}
				}
				if (!((i + 1) % ran)) {
					if (throttle > THROTTLE_LIMIT) {
						if (!emitter->particles[i].bAlive) {
							emitter->particles[i].bAlive = TRUE;
							emitter->particles[i].scl = 1.0f;
						}
						ran = GET_RANDOM(1, 10);
					}
				}
				else
					unlit = true;

			}
			else
				unlit = true;

			if(unlit &&emitter->particles[i].bAlive){
				emitter->particles[i].scl -= 0.25f * D3D::fAnimationRate;
				emitter->particles[i].pos.y = GR_FAR_Z * 0.25f;
				if (player->pGame->gameLevel == LEVEL_HARD) {
					emitter->particles[i].pos.y += player->srt.pos.y;
				}

				if (emitter->particles[i].scl < 0.0f)
					emitter->particles[i].bAlive = FALSE;
			}

		}
		if (throttle > THROTTLE_LIMIT)
			throttle = 0.0f;
	}

	emitter->Update();
}


