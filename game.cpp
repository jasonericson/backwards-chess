#include "game.h"

#include "sprite.h"
#include "text.h"

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

Texture piece_textures[6][2] =
{
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
    short col, row;
};

GridSquare grid[8][8];
GridSquare test_state[8][8];

struct PieceButton
{
    Piece piece;
    uint32 piece_sprite;
};

PieceButton piece_buttons[7];

struct ClickySquare
{
    short x, y, width, height;
    Action action;
    bool enabled;
    void* data;
};

ClickySquare clicky_squares[70];

short grid_pos_x, grid_pos_y;

Piece held_piece;
uint32 held_sprite;
int16 held_last_col;
int16 held_last_row;

bool panel_white = true;

uint32 white_message_id = 0;
uint32 black_message_id = 0;
enum CheckState
{
    CS_None,
    CS_Check,
    CS_CheckMate,
    CS_StaleMate,
};
CheckState white_check_state;
CheckState black_check_state;
bool in_check = false;

void setup_panel()
{
    panel_white = true;

    const short piece_panel_x = 4;
    short next_panel_y = 160 - 4 - 14;

    for (PieceType p = Piece_King; p > Piece_None; p = (PieceType)(p - 1))
    {
        piece_buttons[p - 1] = { { p, true }, sprite_create(&piece_textures[p - 1][true], piece_panel_x, next_panel_y, 1) };
        clicky_squares[p - 1] = ClickySquare { piece_panel_x, next_panel_y, 14, 14, Action::Action_GrabPiece, true, &piece_buttons[p - 1] };
        next_panel_y = next_panel_y - 2 - 14;
    }
}

void switch_panel_color(bool white)
{
    if (panel_white == white)
        return;

    panel_white = white;

    for (PieceType p = Piece_Pawn; p <= Piece_King; p = (PieceType)(p + 1))
    {
        Sprite* sprite = sprite_find(piece_buttons[p - 1].piece_sprite);
        if (sprite != nullptr)
        {
            sprite->tex = &piece_textures[p - 1][white];
        }
        piece_buttons[p - 1].piece.white = white;
    }
}

void game_init()
{
    grid_pos_x = 160 - 112 - 8;
    grid_pos_y = 160 - 112 - 8;
    sprite_create(&board_tex, grid_pos_x, grid_pos_y, 0);

    setup_panel();

    for (short col = 0; col < 8; ++col)
    {
        for (short row = 0; row < 8; ++row)
        {
            grid[col][row].col = col;
            grid[col][row].row = row;
            clicky_squares[6 + (8 * col + row)] = ClickySquare { (short)(grid_pos_x + col * 14), (short)(grid_pos_y + row * 14), 14, 14, Action::Action_GridSquare, true, &grid[col][row] };
        }
    }

    held_piece.type = Piece_None;
    held_sprite = 0;
}

bool check_for_check(bool white, GridSquare (&state)[8][8])
{
    short king_row = -1;
    short king_col = -1;

    bool found = false;
    for (short row = 0; row < 8; ++row)
    {
        for (short col = 0; col < 8; ++col)
        {
            if (state[col][row].piece.type == Piece_King && state[col][row].piece.white == white)
            {
                king_row = row;
                king_col = col;
                found = true;
                break;
            }
        }

        if (found)
            break;
    }

    if (king_row == -1 || king_col == -1)
        return false;

    // right
    int16 check_col = king_col + 1;
    while (check_col < 8)
    {
        GridSquare* sq = &state[check_col][king_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((check_col - king_col == 1 && sq->piece.type == Piece_King) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Rook)
                {
                    return true;
                }
            }

            break;
        }

        ++check_col;
    }

    // left
    check_col = king_col - 1;
    while (check_col >= 0)
    {
        GridSquare* sq = &state[check_col][king_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((king_col - check_col == 1 && sq->piece.type == Piece_King) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Rook)
                {
                    return true;
                }
            }

            break;
        }

        --check_col;
    }

    // up
    int16 check_row = king_row + 1;
    while (check_row < 8)
    {
        GridSquare* sq = &state[king_col][check_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((check_row - king_row == 1 && sq->piece.type == Piece_King) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Rook)
                {
                    return true;
                }
            }

            break;
        }

        ++check_row;
    }

    // down
    check_row = king_row - 1;
    while (check_row >= 0)
    {
        GridSquare* sq = &state[king_col][check_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((king_row - check_row == 1 && sq->piece.type == Piece_King) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Rook)
                {
                    return true;
                }
            }

            break;
        }

        --check_row;
    }

    // right-up
    check_col = king_col + 1;
    check_row = king_row + 1;
    while (check_col < 8 && check_row < 8)
    {
        GridSquare* sq = &state[check_col][check_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((check_col - king_col == 1 && (sq->piece.type == Piece_King || sq->piece.type == Piece_Pawn)) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Bishop)
                {
                    return true;
                }
            }

            break;
        }

        ++check_col;
        ++check_row;
    }

    // right-down
    check_col = king_col + 1;
    check_row = king_row - 1;
    while (check_col < 8 && check_row >= 0)
    {
        GridSquare* sq = &state[check_col][check_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((check_col - king_col == 1 && (sq->piece.type == Piece_King || sq->piece.type == Piece_Pawn)) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Bishop)
                {
                    return true;
                }
            }

            break;
        }

        ++check_col;
        --check_row;
    }

    // left-down
    check_col = king_col - 1;
    check_row = king_row - 1;
    while (check_col >= 0 && check_row >= 0)
    {
        GridSquare* sq = &state[check_col][check_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((king_col - check_col == 1 && (sq->piece.type == Piece_King || sq->piece.type == Piece_Pawn)) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Bishop)
                {
                    return true;
                }
            }

            break;
        }

        --check_col;
        --check_row;
    }

    // left-up
    check_col = king_col - 1;
    check_row = king_row + 1;
    while (check_col >= 0 && check_row < 8)
    {
        GridSquare* sq = &state[check_col][check_row];
        if (sq->piece.type != Piece_None)
        {
            if (sq->piece.white != white)
            {
                if ((king_col - check_col == 1 && (sq->piece.type == Piece_King || sq->piece.type == Piece_Pawn)) || sq->piece.type == Piece_Queen || sq->piece.type == Piece_Bishop)
                {
                    return true;
                }
            }

            break;
        }

        --check_col;
        ++check_row;
    }

    // knight right
    check_col = king_col + 2;
    if (check_col < 8)
    {
        // up
        check_row = king_row + 1;
        if (check_row < 8)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }

        // down
        check_row = king_row - 1;
        if (check_row >= 0)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }
    }

    // knight left
    check_col = king_col - 2;
    if (check_col >= 0)
    {
        // up
        check_row = king_row + 1;
        if (check_row < 8)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }

        // down
        check_row = king_row - 1;
        if (check_row >= 0)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }
    }

    // knight up
    check_row = king_row + 2;
    if (check_row < 8)
    {
        // right
        check_col = king_col + 1;
        if (check_col < 8)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }

        // left
        check_col = king_col - 1;
        if (check_col >= 0)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }
    }

    // knight down
    check_row = king_row - 2;
    if (check_row >= 0)
    {
        // right
        check_col = king_col + 1;
        if (check_col < 8)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }

        // left
        check_col = king_col - 1;
        if (check_col >= 0)
        {
            GridSquare* sq = &state[check_col][check_row];
            if (sq->piece.type == Piece_Knight && sq->piece.white != white)
            {
                return true;
            }
        }
    }

    return false;
}

void test_add(Piece piece, uint16 col, uint16 row)
{
    SDL_memcpy(test_state, grid, sizeof(GridSquare) * 8 * 8);
    test_state[col][row].piece = piece;
}

void test_move(uint16 src_col, uint16 src_row, uint16 dst_col, uint16 dst_row)
{
    SDL_memcpy(test_state, grid, sizeof(GridSquare) * 8 * 8);
    test_state[dst_col][dst_row].piece = test_state[src_col][src_row].piece;
    test_state[src_col][src_row].piece.type = Piece_None;
}

bool check_for_safe_moves(bool white)
{
    int piece_count = 0;
    for (uint16 col = 0; col < 8; ++col)
    {
        for (uint16 row = 0; row < 8; ++row)
        {
            if (grid[col][row].piece.type != Piece_None && grid[col][row].piece.white == white)
            {
                ++piece_count;
                int16 check_col, check_row;

                if (grid[col][row].piece.type == Piece_King)
                {
                    // right
                    check_col = col + 1;
                    if (check_col < 8 && (grid[check_col][row].piece.type == Piece_None || grid[check_col][row].piece.white != white))
                    {
                        test_move(col, row, check_col, row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // left
                    check_col = col - 1;
                    if (check_col >= 0 && (grid[check_col][row].piece.type == Piece_None || grid[check_col][row].piece.white != white))
                    {
                        test_move(col, row, check_col, row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // up
                    check_row = row + 1;
                    if (check_row < 8 && (grid[col][check_row].piece.type == Piece_None || grid[col][check_row].piece.white != white))
                    {
                        test_move(col, row, col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // down
                    check_row = row - 1;
                    if (check_row >= 0 && (grid[col][check_row].piece.type == Piece_None || grid[col][check_row].piece.white != white))
                    {
                        test_move(col, row, col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // right-up
                    check_col = col + 1;
                    check_row = row + 1;
                    if (check_col < 8 && check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // right-down
                    check_col = col + 1;
                    check_row = row - 1;
                    if (check_col < 8 && check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // left-down
                    check_col = col - 1;
                    check_row = row - 1;
                    if (check_col >= 0 && check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // left-up
                    check_col = col - 1;
                    check_row = row + 1;
                    if (check_col >= 0 && check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }
                }
                else if (grid[col][row].piece.type == Piece_Pawn)
                {
                    // right
                    check_col = col + 1;
                    if (check_col < 8 && grid[check_col][row].piece.type == Piece_None)
                    {
                        test_move(col, row, check_col, row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // left
                    check_col = col - 1;
                    if (check_col >= 0 && grid[check_col][row].piece.type == Piece_None)
                    {
                        test_move(col, row, check_col, row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // up
                    check_row = row + 1;
                    if (check_row < 8 && grid[col][check_row].piece.type == Piece_None)
                    {
                        test_move(col, row, col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // down
                    check_row = row - 1;
                    if (check_row >= 0 && grid[col][check_row].piece.type == Piece_None)
                    {
                        test_move(col, row, col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // right-up
                    check_col = col + 1;
                    check_row = row + 1;
                    if (check_col < 8 && check_row < 8 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // right-down
                    check_col = col + 1;
                    check_row = row - 1;
                    if (check_col < 8 && check_row >= 0 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // left-down
                    check_col = col - 1;
                    check_row = row - 1;
                    if (check_col >= 0 && check_row >= 0 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }

                    // left-up
                    check_col = col - 1;
                    check_row = row + 1;
                    if (check_col >= 0 && check_row < 8 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (check_for_check(white, test_state) == false)
                            return true;
                    }
                }
                else if (grid[col][row].piece.type == Piece_Knight)
                {
                    // knight right
                    check_col = col + 2;
                    if (check_col < 8)
                    {
                        // right-up
                        check_row = row + 1;
                        if (check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }

                        // right-down
                        check_row = row - 1;
                        if (check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }
                    }

                    // knight left
                    check_col = col - 2;
                    if (check_col >= 0)
                    {
                        // left-up
                        check_row = row + 1;
                        if (check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }

                        // left-down
                        check_row = row - 1;
                        if (check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }
                    }

                    // knight up
                    check_row = row + 2;
                    if (check_row < 8)
                    {
                        // up-right
                        check_col = col + 1;
                        if (check_col < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }

                        // up-left
                        check_col = col - 1;
                        if (check_col >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }
                    }

                    // knight down
                    check_row = row - 2;
                    if (check_row >= 0)
                    {
                        // down-right
                        check_col = col + 1;
                        if (check_col < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }

                        // down-left
                        check_col = col - 1;
                        if (check_col >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (check_for_check(white, test_state) == false)
                                return true;
                        }
                    }
                }
                else
                {
                    // orthogonal movers (queen & rook)
                    if (grid[col][row].piece.type == Piece_Queen || grid[col][row].piece.type == Piece_Rook)
                    {
                        // right
                        check_col = col + 1;
                        while (check_col < 8)
                        {
                            if (grid[check_col][row].piece.type == Piece_None)
                            {
                                test_move(col, row, check_col, row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][row].piece.white != white)
                                {
                                    test_move(col, row, check_col, row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            ++check_col;
                        }
                        
                        // left
                        check_col = col - 1;
                        while (check_col >= 0)
                        {
                            if (grid[check_col][row].piece.type == Piece_None)
                            {
                                test_move(col, row, check_col, row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][row].piece.white != white)
                                {
                                    test_move(col, row, check_col, row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            --check_col;
                        }
                        
                        // up
                        check_row = row + 1;
                        while (check_row < 8)
                        {
                            if (grid[col][check_row].piece.type == Piece_None)
                            {
                                test_move(col, row, col, check_row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[col][check_row].piece.white != white)
                                {
                                    test_move(col, row, col, check_row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            ++check_row;
                        }
                        
                        // down
                        check_row = row - 1;
                        while (check_row >= 0)
                        {
                            if (grid[col][check_row].piece.type == Piece_None)
                            {
                                test_move(col, row, col, check_row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[col][check_row].piece.white != white)
                                {
                                    test_move(col, row, col, check_row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            --check_row;
                        }
                    }

                    // diagonal movers (queen & bishop)
                    if (grid[col][row].piece.type == Piece_Queen || grid[col][row].piece.type == Piece_Bishop)
                    {
                        // right-up
                        check_col = col + 1;
                        check_row = row + 1;
                        while (check_col < 8 && check_row < 8)
                        {
                            if (grid[check_col][check_row].piece.type == Piece_None)
                            {
                                test_move(col, row, check_col, check_row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            ++check_col;
                            ++check_row;
                        }
                        
                        // right-down
                        check_col = col + 1;
                        check_row = row - 1;
                        while (check_col < 8 && check_row >= 0)
                        {
                            if (grid[check_col][check_row].piece.type == Piece_None)
                            {
                                test_move(col, row, check_col, check_row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            ++check_col;
                            --check_row;
                        }
                        
                        // left-down
                        check_col = col - 1;
                        check_row = row - 1;
                        while (check_col >= 0 && check_row >= 0)
                        {
                            if (grid[check_col][check_row].piece.type == Piece_None)
                            {
                                test_move(col, row, check_col, check_row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            --check_col;
                            --check_row;
                        }
                        
                        // left-up
                        check_col = col - 1;
                        check_row = row + 1;
                        while (check_col >= 0 && check_row < 8)
                        {
                            if (grid[check_col][check_row].piece.type == Piece_None)
                            {
                                test_move(col, row, check_col, check_row);
                                if (check_for_check(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (check_for_check(white, test_state) == false)
                                        return true;
                                }

                                break;
                            }

                            --check_col;
                            ++check_row;
                        }
                    }
                }
            }
        }
    }

    if (piece_count > 0)
        return false;
    else
        return true;
}

uint16 turn = 0;
bool new_turn = true;

void set_sprite_enabled(uint32 id, bool enabled)
{
    Sprite* sprite = sprite_find(id);
    if (sprite != nullptr)
    {
        if (enabled)
        {
            sprite->r = 1.0f;
            sprite->g = 1.0f;
            sprite->b = 1.0f;
        }
        else
        {
            sprite->r = 0.5f;
            sprite->g = 0.5f;
            sprite->b = 0.5f;
        }
    }
}

void game_update()
{
    // if (new_turn)
    // {
    //     if (turn == 0)
    //     {
    //         set_sprite_enabled(queen_panel_sprite, false);
    //         set_sprite_enabled(bishop_panel_sprite, false);
    //         set_sprite_enabled(knight_panel_sprite, false);
    //         set_sprite_enabled()
    //     }
    // }

    if (g_space_down)
    {
        switch_panel_color(!panel_white);
    }

    bool hovering = false;
    for (int i = 0; i < 70; ++i)
    {
        ClickySquare sq = clicky_squares[i];
        if (g_mouse_x >= sq.x && g_mouse_x < sq.x + sq.width && g_mouse_y >= sq.y && g_mouse_y < sq.y + sq.height)
        {
            switch (sq.action)
            {
            case Action_GrabPiece:
                hovering = true;
                if (g_mouse_down)
                {
                    PieceButton* btn = (PieceButton*)sq.data;
                    // grab piece from panel
                    if (!(held_piece.type == btn->piece.type && held_piece.white == btn->piece.white))
                    {
                        Texture* piece_tex = &piece_textures[btn->piece.type - 1][btn->piece.white];

                        if (held_sprite != 0)
                        {
                            sprite_delete(held_sprite);
                        }
                        held_sprite = sprite_create(piece_tex, g_mouse_x, g_mouse_y, 2);
                        held_piece = btn->piece;
                        held_last_col = -1;
                        held_last_row = -1;
                    }
                }
                break;
            case Action_GridSquare:
                {
                    GridSquare* grid_sq = (GridSquare*)sq.data;
                    // place on square
                    if (held_piece.type != Piece_None)
                    {
                        hovering = true;
                        if (g_mouse_down || g_mouse_long_up)
                        {
                            // if space is blank OR has opponent piece
                            if (grid_sq->piece.type == Piece_None || grid_sq->piece.white != held_piece.white)
                            {
                                // get rid of opponent piece
                                if (grid_sq->piece_sprite != 0)
                                {
                                    sprite_delete(grid_sq->piece_sprite);
                                }

                                grid_sq->piece = held_piece;
                                grid_sq->piece_sprite = held_sprite;

                                sprite_set_pos(held_sprite, grid_pos_x + grid_sq->col * 14, grid_pos_y + grid_sq->row * 14);
                                held_piece.type = Piece_None;
                                held_sprite = 0;

                                held_last_col = -1;
                                held_last_row = -1;

                                // white
                                {
                                    bool now_in_check = check_for_check(true, grid);
                                    bool any_safe_moves = check_for_safe_moves(true);
                                    if (now_in_check && !any_safe_moves)
                                    {
                                        if (white_check_state != CS_CheckMate)
                                        {
                                            if (white_message_id != 0)
                                                text_delete(white_message_id);
                                            white_message_id = text_create("Checkmate!", 40, 20, Align_Center);

                                            white_check_state = CS_CheckMate;
                                        }
                                    }
                                    else if (now_in_check && any_safe_moves)
                                    {
                                        if (white_check_state != CS_Check)
                                        {
                                            if (white_message_id != 0)
                                                text_delete(white_message_id);
                                            white_message_id = text_create("Check", 40, 20, Align_Center);

                                            white_check_state = CS_Check;
                                        }
                                    }
                                    else if (!now_in_check && !any_safe_moves)
                                    {
                                        if (white_check_state != CS_StaleMate)
                                        {
                                            if (white_message_id != 0)
                                                text_delete(white_message_id);
                                            white_message_id = text_create("Stalemate", 40, 20, Align_Center);

                                            white_check_state = CS_StaleMate;
                                        }
                                    }
                                    else
                                    {
                                        if (white_check_state != CS_None)
                                        {
                                            if (white_message_id != 0)
                                                text_delete(white_message_id);
                                            white_message_id = 0;

                                            white_check_state = CS_None;
                                        }
                                    }
                                }

                                // black
                                {
                                    bool now_in_check = check_for_check(false, grid);
                                    bool any_safe_moves = check_for_safe_moves(false);
                                    if (now_in_check && !any_safe_moves)
                                    {
                                        if (black_check_state != CS_CheckMate)
                                        {
                                            if (black_message_id != 0)
                                                text_delete(black_message_id);
                                            black_message_id = text_create("Checkmate!", 120, 20, Align_Center, 0.0f, 0.0f, 0.0f);

                                            black_check_state = CS_CheckMate;
                                        }
                                    }
                                    else if (now_in_check && any_safe_moves)
                                    {
                                        if (black_check_state != CS_Check)
                                        {
                                            if (black_message_id != 0)
                                                text_delete(black_message_id);
                                            black_message_id = text_create("Check", 120, 20, Align_Center, 0.0f, 0.0f, 0.0f);

                                            black_check_state = CS_Check;
                                        }
                                    }
                                    else if (!now_in_check && !any_safe_moves)
                                    {
                                        if (black_check_state != CS_StaleMate)
                                        {
                                            if (black_message_id != 0)
                                                text_delete(black_message_id);
                                            black_message_id = text_create("Stalemate", 120, 20, Align_Center, 0.0f, 0.0f, 0.0f);

                                            black_check_state = CS_StaleMate;
                                        }
                                    }
                                    else
                                    {
                                        if (black_check_state != CS_None)
                                        {
                                            if (black_message_id != 0)
                                                text_delete(black_message_id);
                                            black_message_id = 0;

                                            black_check_state = CS_None;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // pick up from square
                    else if (held_piece.type == Piece_None && grid[grid_sq->col][grid_sq->row].piece.type != Piece_None)
                    {
                        hovering = true;
                        if (g_mouse_down)
                        {
                            held_piece = grid[grid_sq->col][grid_sq->row].piece;
                            held_sprite = grid[grid_sq->col][grid_sq->row].piece_sprite;
                            sprite_set_layer(held_sprite, 2);

                            held_last_col = grid_sq->col;
                            held_last_row = grid_sq->row;

                            grid[grid_sq->col][grid_sq->row].piece.type = Piece_None;
                            grid[grid_sq->col][grid_sq->row].piece_sprite = 0;
                        }
                    }
                }
                break;
            default:
                hovering = true;
                break;
            }

            break;
        }
    }

    if (hovering)
    {
        cursor_set(true);
    }
    else
    {
        cursor_set(false);
        if (g_mouse_down || g_mouse_long_up)
        {
            held_piece.type = Piece_None;
            if (held_sprite != 0)
                sprite_delete(held_sprite);
            held_sprite = 0;
        }
    }

    if (held_sprite != 0)
    {
        sprite_set_pos(held_sprite, g_mouse_raw_x - 28, g_mouse_raw_y - 28, true);
    }
}

void game_cleanup()
{

}
