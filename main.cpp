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

enum Action
{
    Action_None,
    Action_GrabPiece,
    Action_GridSquare,
};

enum Piece
{
    Piece_None,
    Piece_Pawn,
    Piece_Rook,
    Piece_Knight,
    Piece_Bishop,
    Piece_Queen,
    Piece_King,
};

struct GridSquare
{
    Piece piece;
    bool white;
    uint32 piece_sprite;
};

GridSquare grid[8][8];

struct ClickySquare
{
    short x, y, width, height;
    Action action;
    Piece piece;
    short grid_col, grid_row;
};

ClickySquare clicky_squares[70];

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

    SDL_Cursor* hover_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    short grid_pos_x = 160 - 112 - 8;
    short grid_pos_y = 160 - 112 - 8;
    sprite_create(&board_tex, grid_pos_x, grid_pos_y);

    short piece_panel_x = 4;
    short king_panel_y = 160 - 4 - 14;
    short queen_panel_y = king_panel_y - 2 - 14;
    short bishop_panel_y = queen_panel_y - 2 - 14;
    short knight_panel_y = bishop_panel_y - 2 - 14;
    short rook_panel_y = knight_panel_y - 2 - 14;
    short pawn_panel_y = rook_panel_y - 2 - 14;

    sprite_create(&king_white_tex, piece_panel_x, king_panel_y);
    sprite_create(&queen_white_tex, piece_panel_x, queen_panel_y);
    sprite_create(&bishop_white_tex, piece_panel_x, bishop_panel_y);
    sprite_create(&knight_white_tex, piece_panel_x, knight_panel_y);
    sprite_create(&rook_white_tex, piece_panel_x, rook_panel_y);
    sprite_create(&pawn_white_tex, piece_panel_x, pawn_panel_y);

    clicky_squares[0] = ClickySquare{ piece_panel_x, king_panel_y, 14, 14, Action::Action_GrabPiece, Piece::Piece_King };
    clicky_squares[1] = ClickySquare{ piece_panel_x, queen_panel_y, 14, 14, Action::Action_GrabPiece, Piece::Piece_Queen };
    clicky_squares[2] = ClickySquare{ piece_panel_x, bishop_panel_y, 14, 14, Action::Action_GrabPiece, Piece::Piece_Bishop };
    clicky_squares[3] = ClickySquare{ piece_panel_x, knight_panel_y, 14, 14, Action::Action_GrabPiece, Piece::Piece_Knight };
    clicky_squares[4] = ClickySquare{ piece_panel_x, rook_panel_y, 14, 14, Action::Action_GrabPiece, Piece::Piece_Rook };
    clicky_squares[5] = ClickySquare{ piece_panel_x, pawn_panel_y, 14, 14, Action::Action_GrabPiece, Piece::Piece_Pawn };

    for (short col = 0; col < 8; ++col)
    {
        for (short row = 0; row < 8; ++row)
        {
            clicky_squares[6 + (8 * col + row)] = ClickySquare{ grid_pos_x + col * 14, grid_pos_y + row * 14, 14, 14, Action::Action_GridSquare, Piece::Piece_None, col, row };
        }
    }

    Piece held_piece = Piece::Piece_None;
    uint32 held_sprite = 0;

    bool quit = false;
    while (!quit)
    {
        Uint64 start = SDL_GetPerformanceCounter();

        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        mouse_x = mouse_x / 4;
        mouse_y = (640 - mouse_y) / 4;

        bool mouse_down = false;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                mouse_down = true;
            }
        }

        bool hovering = false;
        for (int i = 0; i < 70; ++i)
        {
            ClickySquare sq = clicky_squares[i];
            if (mouse_x > sq.x && mouse_x < sq.x + sq.width && mouse_y > sq.y && mouse_y < sq.y + sq.height)
            {
                hovering = true;
                SDL_SetCursor(hover_cursor);
                if (mouse_down)
                {
                    switch (sq.action)
                    {
                    case Action_GrabPiece:
                        if (held_piece != sq.piece)
                        {
                            Texture* piece_tex;
                            switch (sq.piece)
                            {
                            case Piece::Piece_Pawn:
                                piece_tex = &pawn_white_tex;
                                break;
                            case Piece::Piece_Rook:
                                piece_tex = &rook_white_tex;
                                break;
                            case Piece::Piece_Knight:
                                piece_tex = &knight_white_tex;
                                break;
                            case Piece::Piece_Bishop:
                                piece_tex = &bishop_white_tex;
                                break;
                            case Piece::Piece_Queen:
                                piece_tex = &queen_white_tex;
                                break;
                            case Piece::Piece_King:
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
                                held_sprite = sprite_create(piece_tex, mouse_x, mouse_y);
                                held_piece = sq.piece;
                            }
                        }
                        break;
                    case Action_GridSquare:
                        if (held_piece != Piece_None && grid[sq.grid_col][sq.grid_row].piece == Piece_None)
                        {
                            grid[sq.grid_col][sq.grid_row].piece = held_piece;
                            grid[sq.grid_col][sq.grid_row].piece_sprite = held_sprite;

                            sprite_set_pos(held_sprite, grid_pos_x + sq.grid_col * 14, grid_pos_y + sq.grid_row * 14);
                            held_piece = Piece_None;
                            held_sprite = 0;
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
            sprite_set_pos(held_sprite, mouse_x - 7, mouse_y - 7);
        }

        render_update();

        Uint64 end = SDL_GetPerformanceCounter();
        double elapsed_ms = ((end - start) / (double)SDL_GetPerformanceFrequency()) * 1000.0f;
        SDL_Delay(floor(fmax((1000.0 / 60.0) - elapsed_ms, 0.0)));
    }

    render_cleanup();

    SDL_FreeCursor(hover_cursor);

    SDL_Quit();

    return 0;
}
