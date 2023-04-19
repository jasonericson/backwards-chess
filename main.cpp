#include "main.h"

#include "render.h"
#include "sprite.h"

#include <SDL.h>
#include <stdlib.h>
#include <time.h>

Texture board_tex = Texture{ 112, 112, 0.5f, 0.625, 0.375f, 0.5f };
Texture pawn_white_tex = Texture  { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture pawn_black_tex = Texture  { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture rook_white_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture rook_black_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture knight_white_tex = Texture{ 14, 14, 33.0f / 64.0f, 47.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture knight_black_tex = Texture{ 14, 14, 33.0f / 64.0f, 47.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture bishop_white_tex = Texture{ 14, 14, 49.0f / 64.0f, 63.0f / 64.0f, 34.0f / 64.0f, 0.75f };
Texture bishop_black_tex = Texture{ 14, 14, 49.0f / 64.0f, 63.0f / 64.0f, 2.0f / 64.0f, 0.25f };
Texture queen_white_tex = Texture { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 50.0f / 64.0f, 1.0f };
Texture queen_black_tex = Texture { 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 18.0f / 64.0f, 0.5f };
Texture king_white_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 50.0f / 64.0f, 1.0f };
Texture king_black_tex = Texture  { 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 18.0f / 64.0f, 0.5f };

uint32 grid_sprites[8][8] = { 0 };

enum Interaction
{
    None,
    GrabPiece,
    GridSquare,
};

enum Piece
{
    Piece_None,
    Pawn,
    Rook,
    Knight,
    Bishop,
    Queen,
    King,
};

struct ClickySquare
{
    short x, y, width, height;
    Interaction interaction;
    Piece piece;
};

ClickySquare clicky_squares[6];

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    render_init();
    sprite_init();

    float r = 1.0f;
    float g = 0.0f;
    float b = 1.0f;

    time_t t;
    srand((unsigned) time(&t));

    SDL_Cursor* hoverCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    short gridPosX = 160 - 112 - 8;
    short gridPosY = 160 - 112 - 8;
    sprite_create(&board_tex, gridPosX, gridPosY);

    short piecePanelX = 4;
    short kingPanelY = 160 - 4 - 14;
    short queenPanelY = kingPanelY - 2 - 14;
    short bishopPanelY = queenPanelY - 2 - 14;
    short knightPanelY = bishopPanelY - 2 - 14;
    short rookPanelY = knightPanelY - 2 - 14;
    short pawnPanelY = rookPanelY - 2 - 14;

    sprite_create(&king_white_tex, piecePanelX, kingPanelY);
    sprite_create(&queen_white_tex, piecePanelX, queenPanelY);
    sprite_create(&bishop_white_tex, piecePanelX, bishopPanelY);
    sprite_create(&knight_white_tex, piecePanelX, knightPanelY);
    sprite_create(&rook_white_tex, piecePanelX, rookPanelY);
    sprite_create(&pawn_white_tex, piecePanelX, pawnPanelY);

    clicky_squares[0] = ClickySquare{ piecePanelX, kingPanelY, 14, 14, Interaction::GrabPiece, Piece::King };
    clicky_squares[1] = ClickySquare{ piecePanelX, queenPanelY, 14, 14, Interaction::GrabPiece, Piece::Queen };
    clicky_squares[2] = ClickySquare{ piecePanelX, bishopPanelY, 14, 14, Interaction::GrabPiece, Piece::Bishop };
    clicky_squares[3] = ClickySquare{ piecePanelX, knightPanelY, 14, 14, Interaction::GrabPiece, Piece::Knight };
    clicky_squares[4] = ClickySquare{ piecePanelX, rookPanelY, 14, 14, Interaction::GrabPiece, Piece::Rook };
    clicky_squares[5] = ClickySquare{ piecePanelX, pawnPanelY, 14, 14, Interaction::GrabPiece, Piece::Pawn };

    Piece held_piece = Piece::Piece_None;
    uint32 held_sprite = 0;

    bool quit = false;
    while (!quit)
    {
        Uint64 start = SDL_GetPerformanceCounter();

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        mouseX = mouseX / 4;
        mouseY = (640 - mouseY) / 4;

        bool mouseDown = false;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                mouseDown = true;
            }
        }

        bool hovering = false;
        for (int i = 0; i < 6; ++i)
        {
            ClickySquare sq = clicky_squares[i];
            if (mouseX > sq.x && mouseX < sq.x + sq.width && mouseY > sq.y && mouseY < sq.y + sq.height)
            {
                hovering = true;
                SDL_SetCursor(hoverCursor);
                if (mouseDown)
                {
                    switch (sq.interaction)
                    {
                    case GrabPiece:
                        if (held_piece != sq.piece)
                        {
                            Texture* piece_tex;
                            switch (sq.piece)
                            {
                            case Piece::Pawn:
                                piece_tex = &pawn_white_tex;
                                break;
                            case Piece::Rook:
                                piece_tex = &rook_white_tex;
                                break;
                            case Piece::Knight:
                                piece_tex = &knight_white_tex;
                                break;
                            case Piece::Bishop:
                                piece_tex = &bishop_white_tex;
                                break;
                            case Piece::Queen:
                                piece_tex = &queen_white_tex;
                                break;
                            case Piece::King:
                                piece_tex = &king_white_tex;
                                break;
                            default:
                                piece_tex = nullptr;
                                break;
                            }

                            if (piece_tex != nullptr)
                            {
                                if (held_sprite != 0)
                                {
                                    sprite_delete(held_sprite);
                                }
                                held_sprite = sprite_create(piece_tex, mouseX, mouseY);
                                held_piece = sq.piece;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }

                break;
            }

            if (!hovering)
            {
                SDL_SetCursor(SDL_GetDefaultCursor());
            }
        }

        if (held_sprite != 0)
        {
            sprite_set_pos(held_sprite, mouseX - 7, mouseY - 7);
        }

        render_update();

        Uint64 end = SDL_GetPerformanceCounter();
        double elapsedMs = ((end - start) / (double)SDL_GetPerformanceFrequency()) * 1000.0f;
        SDL_Delay(floor(fmax((1000.0 / 60.0) - elapsedMs, 0.0)));
    }

    render_cleanup();

    SDL_FreeCursor(hoverCursor);

    SDL_Quit();

    return 0;
}
