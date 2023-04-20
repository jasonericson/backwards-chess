#include "game.h"

#include "sprite.h"

#include <SDL.h>

Texture board_tex = Texture{ 0, 112, 112, 0.5f, 0.625, 0.375f, 0.5f };

uint32 grid_sprites[8][8] = { 0 };

enum Action
{
    Action_None,
    Action_GrabPiece,
    Action_GridSquare,
    Action_FlipColorButton,
};

enum PieceType
{
    Piece_None,
    Piece_Pawn,
    Piece_Rook,
    Piece_Knight,
    Piece_Bishop,
    Piece_Queen,
    Piece_King,
};

struct Piece
{
    PieceType type;
    bool white;
};

Texture piece_textures[7][2] =
{
    {{}, {}},
    /* pawn */   { Texture{ 0, 14, 14,  1.0f / 64.0f, 15.0f / 64.0f,  2.0f / 64.0f, 0.25f }, Texture{ 0, 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 34.0f / 64.0f, 0.75f }},
    /* rook */   { Texture{ 0, 14, 14, 17.0f / 64.0f, 31.0f / 64.0f,  2.0f / 64.0f, 0.25f }, Texture{ 0, 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 34.0f / 64.0f, 0.75f }},
    /* knight */ { Texture{ 0, 14, 14, 33.0f / 64.0f, 47.0f / 64.0f,  2.0f / 64.0f, 0.25f }, Texture{ 0, 14, 14, 33.0f / 64.0f, 47.0f / 64.0f, 34.0f / 64.0f, 0.75f }},
    /* bishop */ { Texture{ 0, 14, 14, 49.0f / 64.0f, 63.0f / 64.0f,  2.0f / 64.0f, 0.25f }, Texture{ 0, 14, 14, 49.0f / 64.0f, 63.0f / 64.0f, 34.0f / 64.0f, 0.75f }},
    /* queen */  { Texture{ 0, 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 18.0f / 64.0f, 0.50f }, Texture{ 0, 14, 14,  1.0f / 64.0f, 15.0f / 64.0f, 50.0f / 64.0f,  1.0f }},
    /* king */   { Texture{ 0, 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 18.0f / 64.0f, 0.50f }, Texture{ 0, 14, 14, 17.0f / 64.0f, 31.0f / 64.0f, 50.0f / 64.0f,  1.0f }},
};

struct GridSquare
{
    Piece piece;
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

short grid_pos_x, grid_pos_y;

Piece held_piece;
uint32 held_sprite;

uint32 king_panel_sprite, queen_panel_sprite, bishop_panel_sprite, knight_panel_sprite, rook_panel_sprite, pawn_panel_sprite;
bool panel_white = true;

void setup_panel(bool white)
{
    if (panel_white == white)
        return;

    panel_white = white;

    const short piece_panel_x = 4;
    const short king_panel_y = 160 - 4 - 14;
    const short queen_panel_y = king_panel_y - 2 - 14;
    const short bishop_panel_y = queen_panel_y - 2 - 14;
    const short knight_panel_y = bishop_panel_y - 2 - 14;
    const short rook_panel_y = knight_panel_y - 2 - 14;
    const short pawn_panel_y = rook_panel_y - 2 - 14;

    sprite_delete(king_panel_sprite);
    sprite_delete(queen_panel_sprite);
    sprite_delete(bishop_panel_sprite);
    sprite_delete(knight_panel_sprite);
    sprite_delete(rook_panel_sprite);
    sprite_delete(pawn_panel_sprite);

    king_panel_sprite = sprite_create(&piece_textures[Piece_King][white], piece_panel_x, king_panel_y);
    queen_panel_sprite = sprite_create(&piece_textures[Piece_Queen][white], piece_panel_x, queen_panel_y);
    bishop_panel_sprite = sprite_create(&piece_textures[Piece_Bishop][white], piece_panel_x, bishop_panel_y);
    knight_panel_sprite = sprite_create(&piece_textures[Piece_Knight][white], piece_panel_x, knight_panel_y);
    rook_panel_sprite = sprite_create(&piece_textures[Piece_Rook][white], piece_panel_x, rook_panel_y);
    pawn_panel_sprite = sprite_create(&piece_textures[Piece_Pawn][white], piece_panel_x, pawn_panel_y);

    clicky_squares[0] = ClickySquare { piece_panel_x, king_panel_y, 14, 14, Action::Action_GrabPiece, { PieceType::Piece_King, white } };
    clicky_squares[1] = ClickySquare { piece_panel_x, queen_panel_y, 14, 14, Action::Action_GrabPiece, { PieceType::Piece_Queen, white } };
    clicky_squares[2] = ClickySquare{ piece_panel_x, bishop_panel_y, 14, 14, Action::Action_GrabPiece, { PieceType::Piece_Bishop, white } };
    clicky_squares[3] = ClickySquare{ piece_panel_x, knight_panel_y, 14, 14, Action::Action_GrabPiece, { PieceType::Piece_Knight, white } };
    clicky_squares[4] = ClickySquare{ piece_panel_x, rook_panel_y, 14, 14, Action::Action_GrabPiece, { PieceType::Piece_Rook, white } };
    clicky_squares[5] = ClickySquare{ piece_panel_x, pawn_panel_y, 14, 14, Action::Action_GrabPiece, { PieceType::Piece_Pawn, white } };
}

void game_init()
{
    grid_pos_x = 160 - 112 - 8;
    grid_pos_y = 160 - 112 - 8;
    sprite_create(&board_tex, grid_pos_x, grid_pos_y);

    setup_panel(false);

    for (short col = 0; col < 8; ++col)
    {
        for (short row = 0; row < 8; ++row)
        {
            clicky_squares[6 + (8 * col + row)] = ClickySquare { grid_pos_x + col * 14, grid_pos_y + row * 14, 14, 14, Action::Action_GridSquare, {} , col, row };
        }
    }

    held_piece.type = Piece_None;
    held_sprite = 0;
}

void game_update()
{
    if (g_space_down)
    {
        setup_panel(!panel_white);
    }

    bool hovering = false;
    for (int i = 0; i < 70; ++i)
    {
        ClickySquare sq = clicky_squares[i];
        if (g_mouse_x > sq.x && g_mouse_x < sq.x + sq.width && g_mouse_y > sq.y && g_mouse_y < sq.y + sq.height)
        {
            hovering = true;
            cursor_set(true);
            if (g_mouse_down)
            {
                switch (sq.action)
                {
                case Action_GrabPiece:
                    if (!(held_piece.type == sq.piece.type && held_piece.white == sq.piece.white))
                    {
                        Texture* piece_tex = &piece_textures[sq.piece.type][sq.piece.white];

                        if (held_sprite != 0)
                        {
                            sprite_delete(held_sprite);
                        }
                        held_sprite = sprite_create(piece_tex, g_mouse_x, g_mouse_y);
                        held_piece = sq.piece;
                    }
                    break;
                case Action_GridSquare:
                    if (held_piece.type != Piece_None && grid[sq.grid_col][sq.grid_row].piece.type == Piece_None)
                    {
                        grid[sq.grid_col][sq.grid_row].piece = held_piece;
                        grid[sq.grid_col][sq.grid_row].piece_sprite = held_sprite;

                        sprite_set_pos(held_sprite, grid_pos_x + sq.grid_col * 14, grid_pos_y + sq.grid_row * 14);
                        held_piece.type = Piece_None;
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
            cursor_set(false);
        }
    }

    if (held_sprite != 0)
    {
        sprite_set_pos(held_sprite, g_mouse_x - 7, g_mouse_y - 7);
    }
}

void game_cleanup()
{

}
