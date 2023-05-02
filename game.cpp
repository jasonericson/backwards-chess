#include "game.h"

#include "sprite.h"
#include "text.h"

#include <SDL.h>

Texture board_tex = Texture{ MapId_Pieces, 112, 112, 32, 40, 24, 32 };
Texture square_tex = Texture{ MapId_Pieces, 14, 14,  48, 49, 31, 32 };

uint32 grid_sprites[8][8] = { 0 };

enum SpriteLayer
{
    Layer_Board = 0,
    Layer_SquareHighlight = 1,
    Layer_PlacedPiece = 2,
    Layer_HeldPiece = 3,
};

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
    /* pawn */   { Texture{ MapId_Pieces, 14, 14,  1, 15,  2, 16 }, Texture{ MapId_Pieces, 14, 14,  1, 15, 34, 48 }},
    /* rook */   { Texture{ MapId_Pieces, 14, 14, 17, 31,  2, 16 }, Texture{ MapId_Pieces, 14, 14, 17, 31, 34, 48 }},
    /* knight */ { Texture{ MapId_Pieces, 14, 14, 33, 47,  2, 16 }, Texture{ MapId_Pieces, 14, 14, 33, 47, 34, 48 }},
    /* bishop */ { Texture{ MapId_Pieces, 14, 14, 49, 63,  2, 16 }, Texture{ MapId_Pieces, 14, 14, 49, 63, 34, 48 }},
    /* queen */  { Texture{ MapId_Pieces, 14, 14,  1, 15, 18, 32 }, Texture{ MapId_Pieces, 14, 14,  1, 15, 50, 64 }},
    /* king */   { Texture{ MapId_Pieces, 14, 14, 17, 31, 18, 32 }, Texture{ MapId_Pieces, 14, 14, 17, 31, 50, 64 }},
};

struct GridSquare
{
    Piece piece;
    uint32 piece_sprite;
    uint32 overlay_sprite;
    short col, row;
};

GridSquare grid[8][8];
GridSquare test_state[8][8];
Uint64 valid_spaces;

struct PieceButton
{
    Piece piece;
    uint32 piece_sprite;
    uint16 count[2];
    uint32 count_text_id;
    char count_text[3];
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

uint32 preview_sprite;

bool player_white = true;

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
    player_white = true;

    const short piece_panel_x = 4;
    short next_panel_y = 160 - 4 - 14;

    for (PieceType p = Piece_King; p > Piece_None; p = (PieceType)(p - 1))
    {
        piece_buttons[p - 1] = { { p, true }, sprite_create(&piece_textures[p - 1][true], piece_panel_x, next_panel_y, Layer_PlacedPiece) };
        clicky_squares[p - 1] = ClickySquare { piece_panel_x, next_panel_y, 14, 14, Action::Action_GrabPiece, true, &piece_buttons[p - 1] };
        next_panel_y = next_panel_y - 2 - 14;
    }

    piece_buttons[Piece_Pawn - 1].count[0] = 8;
    piece_buttons[Piece_Pawn - 1].count[1] = 8;
    piece_buttons[Piece_Rook - 1].count[0] = 2;
    piece_buttons[Piece_Rook - 1].count[1] = 2;
    piece_buttons[Piece_Knight - 1].count[0] = 2;
    piece_buttons[Piece_Knight - 1].count[1] = 2;
    piece_buttons[Piece_Bishop - 1].count[0] = 2;
    piece_buttons[Piece_Bishop - 1].count[1] = 2;
    piece_buttons[Piece_Queen - 1].count[0] = 1;
    piece_buttons[Piece_Queen - 1].count[1] = 1;
    piece_buttons[Piece_King - 1].count[0] = 1;
    piece_buttons[Piece_King - 1].count[1] = 1;

    const short text_x = piece_panel_x + 14 - 1;

    for (PieceType p = Piece_Pawn; p <= Piece_King; p = (PieceType)(p + 1))
    {
        SDL_snprintf(piece_buttons[p - 1].count_text, 3, "x%d", piece_buttons[p - 1].count[player_white]);
        short y = clicky_squares[p - 1].y;
        piece_buttons[p - 1].count_text_id = text_create(piece_buttons[p - 1].count_text, text_x, y);
    }
}

void switch_panel_color(bool white)
{
    if (player_white == white)
        return;

    player_white = white;

    for (PieceType p = Piece_Pawn; p <= Piece_King; p = (PieceType)(p + 1))
    {
        PieceButton* btn = &piece_buttons[p - 1];
        Sprite* sprite = sprite_find(btn->piece_sprite);
        if (sprite != nullptr)
        {
            sprite->tex = &piece_textures[p - 1][white];
        }

        btn->piece.white = white;

        SDL_snprintf(btn->count_text, 3, "x%d", btn->count[white]);
        text_change(btn->count_text_id, btn->count_text);

        if (white)
            text_set_color(btn->count_text_id, 1.0f, 1.0f, 1.0f);
        else
            text_set_color(btn->count_text_id, 0.0f, 0.0f, 0.0f);
    }
}

void game_init()
{
    grid_pos_x = 160 - 112 - 8;
    grid_pos_y = 160 - 112 - 8;
    sprite_create(&board_tex, grid_pos_x, grid_pos_y, Layer_Board);

    setup_panel();

    for (short col = 0; col < 8; ++col)
    {
        for (short row = 0; row < 8; ++row)
        {
            grid[col][row].piece_sprite = 0;
            grid[col][row].overlay_sprite = 0;
            grid[col][row].col = col;
            grid[col][row].row = row;
            clicky_squares[6 + (8 * col + row)] = ClickySquare { (short)(grid_pos_x + col * 14), (short)(grid_pos_y + row * 14), 14, 14, Action::Action_GridSquare, true, &grid[col][row] };
        }
    }

    held_piece.type = Piece_None;
    held_sprite = 0;
    preview_sprite = 0;
}

bool is_king_in_danger(bool white, GridSquare (&state)[8][8])
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

bool any_safe_moves(bool white)
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
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // left
                    check_col = col - 1;
                    if (check_col >= 0 && (grid[check_col][row].piece.type == Piece_None || grid[check_col][row].piece.white != white))
                    {
                        test_move(col, row, check_col, row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // up
                    check_row = row + 1;
                    if (check_row < 8 && (grid[col][check_row].piece.type == Piece_None || grid[col][check_row].piece.white != white))
                    {
                        test_move(col, row, col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // down
                    check_row = row - 1;
                    if (check_row >= 0 && (grid[col][check_row].piece.type == Piece_None || grid[col][check_row].piece.white != white))
                    {
                        test_move(col, row, col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // right-up
                    check_col = col + 1;
                    check_row = row + 1;
                    if (check_col < 8 && check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // right-down
                    check_col = col + 1;
                    check_row = row - 1;
                    if (check_col < 8 && check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // left-down
                    check_col = col - 1;
                    check_row = row - 1;
                    if (check_col >= 0 && check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // left-up
                    check_col = col - 1;
                    check_row = row + 1;
                    if (check_col >= 0 && check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
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
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // left
                    check_col = col - 1;
                    if (check_col >= 0 && grid[check_col][row].piece.type == Piece_None)
                    {
                        test_move(col, row, check_col, row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // up
                    check_row = row + 1;
                    if (check_row < 8 && grid[col][check_row].piece.type == Piece_None)
                    {
                        test_move(col, row, col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // down
                    check_row = row - 1;
                    if (check_row >= 0 && grid[col][check_row].piece.type == Piece_None)
                    {
                        test_move(col, row, col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // right-up
                    check_col = col + 1;
                    check_row = row + 1;
                    if (check_col < 8 && check_row < 8 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // right-down
                    check_col = col + 1;
                    check_row = row - 1;
                    if (check_col < 8 && check_row >= 0 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // left-down
                    check_col = col - 1;
                    check_row = row - 1;
                    if (check_col >= 0 && check_row >= 0 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
                            return true;
                    }

                    // left-up
                    check_col = col - 1;
                    check_row = row + 1;
                    if (check_col >= 0 && check_row < 8 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != white)
                    {
                        test_move(col, row, check_col, check_row);
                        if (is_king_in_danger(white, test_state) == false)
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
                            if (is_king_in_danger(white, test_state) == false)
                                return true;
                        }

                        // right-down
                        check_row = row - 1;
                        if (check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (is_king_in_danger(white, test_state) == false)
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
                            if (is_king_in_danger(white, test_state) == false)
                                return true;
                        }

                        // left-down
                        check_row = row - 1;
                        if (check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (is_king_in_danger(white, test_state) == false)
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
                            if (is_king_in_danger(white, test_state) == false)
                                return true;
                        }

                        // up-left
                        check_col = col - 1;
                        if (check_col >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (is_king_in_danger(white, test_state) == false)
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
                            if (is_king_in_danger(white, test_state) == false)
                                return true;
                        }

                        // down-left
                        check_col = col - 1;
                        if (check_col >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != white))
                        {
                            test_move(col, row, check_col, check_row);
                            if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][row].piece.white != white)
                                {
                                    test_move(col, row, check_col, row);
                                    if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][row].piece.white != white)
                                {
                                    test_move(col, row, check_col, row);
                                    if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[col][check_row].piece.white != white)
                                {
                                    test_move(col, row, col, check_row);
                                    if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[col][check_row].piece.white != white)
                                {
                                    test_move(col, row, col, check_row);
                                    if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (is_king_in_danger(white, test_state) == false)
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
                                if (is_king_in_danger(white, test_state) == false)
                                    return true;
                            }
                            else
                            {
                                if (grid[check_col][check_row].piece.white != white)
                                {
                                    test_move(col, row, check_col, check_row);
                                    if (is_king_in_danger(white, test_state) == false)
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

void check_for_check(bool white)
{
    CheckState* check_state = white ? &white_check_state : &black_check_state;
    uint32* message_id = white ? &white_message_id : &black_message_id;
    short message_x = white ? 40 : 120;
    float r = white ? 1.0f : 0.0f;
    float g = white ? 1.0f : 0.0f;
    float b = white ? 1.0f : 0.0f;

    bool now_in_check = is_king_in_danger(white, grid);
    bool check_escapable = any_safe_moves(white);
    if (now_in_check && !check_escapable)
    {
        if (*check_state != CS_CheckMate)
        {
            if (*message_id != 0)
                text_delete(*message_id);
            *message_id = text_create("Checkmate!", message_x, 20, Align_Center, Font_Default, r, g, b);

            *check_state = CS_CheckMate;
        }
    }
    else if (now_in_check && check_escapable)
    {
        if (*check_state != CS_Check)
        {
            if (*message_id != 0)
                text_delete(*message_id);
            *message_id = text_create("Check", message_x, 20, Align_Center, Font_Default, r, g, b);

            *check_state = CS_Check;
        }
    }
    else if (!now_in_check && !check_escapable)
    {
        if (*check_state != CS_StaleMate)
        {
            if (*message_id != 0)
                text_delete(*message_id);
            *message_id = text_create("Stalemate", message_x, 20, Align_Center, Font_Default, r, g, b);

            *check_state = CS_StaleMate;
        }
    }
    else
    {
        if (*check_state != CS_None)
        {
            if (*message_id != 0)
                text_delete(*message_id);
            *message_id = 0;

            *check_state = CS_None;
        }
    }
}

void set_valid_space(short col, short row, bool valid)
{
    GridSquare* sq = &grid[col][row];
    if (valid)
    {
        valid_spaces |= 1ULL << (row * 8 + col);
    }
    else
    {
        valid_spaces &= ~(1ULL << (row * 8 + col));
    }
}

void clear_overlays()
{
    for (int16 row = 0; row < 8; ++row)
    {
        for (int16 col = 0; col < 8; ++col)
        {
            if (grid[col][row].overlay_sprite != 0)
                sprite_delete(grid[col][row].overlay_sprite);
            grid[col][row].overlay_sprite = 0;
        }
    }
}

bool is_valid_space(short col, short row)
{
    return (valid_spaces >> (row * 8 + col)) & 1U;
}

void set_all_valid_moves(short col, short row)
{
    valid_spaces = 0;

    short check_col, check_row;
    if (grid[col][row].piece.type == Piece_Pawn)
    {
        // right
        check_col = col + 1;
        if (check_col < 8 && grid[check_col][row].piece.type == Piece_None)
        {
            set_valid_space(check_col, row, true);
        }

        // left
        check_col = col - 1;
        if (check_col >= 0 && grid[check_col][row].piece.type == Piece_None)
        {
            set_valid_space(check_col, row, true);
        }

        // up
        check_row = row + 1;
        if (check_row < 8 && grid[col][check_row].piece.type == Piece_None)
        {
            set_valid_space(col, check_row, true);
        }

        // down
        check_row = row - 1;
        if (check_row >= 0 && grid[col][check_row].piece.type == Piece_None)
        {
            set_valid_space(col, check_row, true);
        }

        // right-up
        check_col = col + 1;
        check_row = row + 1;
        if (check_col < 8 && check_row < 8 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != player_white)
        {
            set_valid_space(check_col, check_row, true);
        }

        // right-down
        check_col = col + 1;
        check_row = row - 1;
        if (check_col < 8 && check_row >= 0 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != player_white)
        {
            set_valid_space(check_col, check_row, true);
        }

        // left-down
        check_col = col - 1;
        check_row = row - 1;
        if (check_col >= 0 && check_row >= 0 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != player_white)
        {
            set_valid_space(check_col, check_row, true);
        }

        // left-up
        check_col = col - 1;
        check_row = row + 1;
        if (check_col >= 0 && check_row < 8 && grid[check_col][check_row].piece.type != Piece_None && grid[check_col][check_row].piece.white != player_white)
        {
            set_valid_space(check_col, check_row, true);
        }
    }
    else if (grid[col][row].piece.type == Piece_King)
    {
        // right
        check_col = col + 1;
        if (check_col < 8 && (grid[check_col][row].piece.type == Piece_None || grid[check_col][row].piece.white != player_white))
        {
            set_valid_space(check_col, row, true);
        }

        // left
        check_col = col - 1;
        if (check_col >= 0 && (grid[check_col][row].piece.type == Piece_None || grid[check_col][row].piece.white != player_white))
        {
            set_valid_space(check_col, row, true);
        }

        // up
        check_row = row + 1;
        if (check_row < 8 && (grid[col][check_row].piece.type == Piece_None || grid[col][check_row].piece.white != player_white))
        {
            set_valid_space(col, check_row, true);
        }

        // down
        check_row = row - 1;
        if (check_row >= 0 && (grid[col][check_row].piece.type == Piece_None || grid[col][check_row].piece.white != player_white))
        {
            set_valid_space(col, check_row, true);
        }

        // right-up
        check_col = col + 1;
        check_row = row + 1;
        if (check_col < 8 && check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
        {
            set_valid_space(check_col, check_row, true);
        }

        // right-down
        check_col = col + 1;
        check_row = row - 1;
        if (check_col < 8 && check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
        {
            set_valid_space(check_col, check_row, true);
        }

        // left-down
        check_col = col - 1;
        check_row = row - 1;
        if (check_col >= 0 && check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
        {
            set_valid_space(check_col, check_row, true);
        }

        // left-up
        check_col = col - 1;
        check_row = row + 1;
        if (check_col >= 0 && check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
        {
            set_valid_space(check_col, check_row, true);
        }
    }
    else if (grid[col][row].piece.type == Piece_Knight)
    {
        // right
        check_col = col + 2;
        if (check_col < 8)
        {
            // right-up
            check_row = row + 1;
            if (check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }

            // right-down
            check_row = row - 1;
            if (check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }
        }

        // left
        check_col = col - 2;
        if (check_col >= 8)
        {
            // left-up
            check_row = row + 1;
            if (check_row < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }

            // left-down
            check_row = row - 1;
            if (check_row >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }
        }

        // up
        check_row = row + 2;
        if (check_row < 8)
        {
            // up-right
            check_col = col + 1;
            if (check_col < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }

            // up-left
            check_col = col - 1;
            if (check_col >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }
        }

        // down
        check_row = row - 2;
        if (check_row >= 0)
        {
            // down-right
            check_col = col + 1;
            if (check_col < 8 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }

            // down-left
            check_col = col - 1;
            if (check_col >= 0 && (grid[check_col][check_row].piece.type == Piece_None || grid[check_col][check_row].piece.white != player_white))
            {
                set_valid_space(check_col, check_row, true);
            }
        }
    }
    else
    {
        // diagonals
        if (grid[col][row].piece.type == Piece_Queen || grid[col][row].piece.type == Piece_Bishop)
        {
            // right-up
            check_col = col + 1;
            check_row = row + 1;
            while (check_col < 8 && check_row < 8)
            {
                if (grid[check_col][check_row].piece.type == Piece_None)
                {
                    set_valid_space(check_col, check_row, true);
                }
                else
                {
                    if (grid[check_col][check_row].piece.white != player_white)
                    {
                        set_valid_space(check_col, check_row, true);
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
                    set_valid_space(check_col, check_row, true);
                }
                else
                {
                    if (grid[check_col][check_row].piece.white != player_white)
                    {
                        set_valid_space(check_col, check_row, true);
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
                    set_valid_space(check_col, check_row, true);
                }
                else
                {
                    if (grid[check_col][check_row].piece.white != player_white)
                    {
                        set_valid_space(check_col, check_row, true);
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
                    set_valid_space(check_col, check_row, true);
                }
                else
                {
                    if (grid[check_col][check_row].piece.white != player_white)
                    {
                        set_valid_space(check_col, check_row, true);
                    }

                    break;
                }

                --check_col;
                ++check_row;
            }
        }

        // orthogonals
        if (grid[col][row].piece.type == Piece_Queen || grid[col][row].piece.type == Piece_Rook)
        {
            // right
            check_col = col + 1;
            while (check_col < 8)
            {
                if (grid[check_col][row].piece.type == Piece_None)
                {
                    set_valid_space(check_col, row, true);
                }
                else
                {
                    if (grid[check_col][row].piece.white != player_white)
                    {
                        set_valid_space(check_col, row, true);
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
                    set_valid_space(check_col, row, true);
                }
                else
                {
                    if (grid[check_col][row].piece.white != player_white)
                    {
                        set_valid_space(check_col, row, true);
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
                    set_valid_space(col, check_row, true);
                }
                else
                {
                    if (grid[col][check_row].piece.white != player_white)
                    {
                        set_valid_space(col, check_row, true);
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
                    set_valid_space(col, check_row, true);
                }
                else
                {
                    if (grid[col][check_row].piece.white != player_white)
                    {
                        set_valid_space(col, check_row, true);
                    }

                    break;
                }

                --check_row;
            }
        }
    }
}

uint16 turn = 0;
bool new_turn = true;
bool turn_move = false;

void set_panel_button_enabled(PieceType piece_type, bool enabled)
{
    ClickySquare* sq = &clicky_squares[piece_type - 1];
    sq->enabled = enabled;
    PieceButton* btn = (PieceButton*)sq->data;
    Sprite* sprite = sprite_find(btn->piece_sprite);
    if (sprite != nullptr)
    {
        if (enabled)
        {
            sprite->a = 1.0f;
        }
        else
        {
            sprite->a = 0.4f;
        }
    }

    if (enabled)
        text_set_alpha(btn->count_text_id, 1.0f);
    else
        text_set_alpha(btn->count_text_id, 0.4f);
}

void highlight_square(uint16 col, uint16 row, bool highlight)
{
    if (highlight)
    {
        if (grid[col][row].overlay_sprite == 0)
            grid[col][row].overlay_sprite = sprite_create(&square_tex, grid_pos_x + col * 14, grid_pos_y + row * 14, Layer_SquareHighlight, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.4f);
    }
    else
    {
        if (grid[col][row].overlay_sprite != 0)
        {
            sprite_delete(grid[col][row].overlay_sprite);
            grid[col][row].overlay_sprite = 0;
        }
    }
}

uint16 last_hover_col = 9;
uint16 last_hover_row = 9;

void game_update()
{
    if (new_turn)
    {
        if (turn == 0)
        {
            for (PieceType p = Piece_Pawn; p < Piece_King; p = (PieceType)(p + 1))
            {
                set_panel_button_enabled(p, false);
            }
        }
        else if (turn == 1)
        {
            switch_panel_color(false);
        }
        else
        {
            uint16 step = (turn - 1) % 4;
            if (step == 0 || step == 2)
            {
                turn_move = true;
                for (PieceType p = Piece_Pawn; p <= Piece_King; p = (PieceType)(p + 1))
                {
                    set_panel_button_enabled(p, false);
                }

                switch_panel_color(step == 0);
            }
            else
            {
                if (turn == 2)
                    switch_panel_color(step == 1);

                turn_move = false;
                for (PieceType p = Piece_Pawn; p <= Piece_King; p = (PieceType)(p + 1))
                {
                    set_panel_button_enabled(p, piece_buttons[p - 1].count[player_white] > 0);
                }
            }
        }

        ++turn;
        new_turn = false;
    }

    bool hovering = false;
    bool show_preview = false;
    bool show_highlights = false;
    bool over_panel = false;
    bool click_used = false;
    for (int i = 0; i < 70; ++i)
    {
        ClickySquare sq = clicky_squares[i];
        if (sq.enabled && g_mouse_x >= sq.x && g_mouse_x < sq.x + sq.width && g_mouse_y >= sq.y && g_mouse_y < sq.y + sq.height)
        {
            switch (sq.action)
            {
            case Action_GrabPiece:
                hovering = true;
                over_panel = true;
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
                        held_sprite = sprite_create(piece_tex, g_mouse_x, g_mouse_y, Layer_HeldPiece);
                        held_piece = btn->piece;
                        held_last_col = -1;
                        held_last_row = -1;

                        valid_spaces = 0;
                        // set valid spaces (in this case, any empty)
                        for (uint16 row = 0; row < 8; ++row)
                        {
                            for (uint16 col = 0; col < 8; ++col)
                            {
                                set_valid_space(col, row, grid[col][row].piece.type == Piece_None);
                            }
                        }

                        click_used = true;
                    }
                }
                break;
            case Action_GridSquare:
                {
                    GridSquare* grid_sq = (GridSquare*)sq.data;

                    // place on square
                    if ((g_mouse_down || g_mouse_long_up) && held_piece.type != Piece_None && is_valid_space(grid_sq->col, grid_sq->row))
                    {
                        if (turn_move)
                        {
                            test_move(held_last_col, held_last_row, grid_sq->col, grid_sq->row);
                        }
                        else
                        {
                            test_add(held_piece, grid_sq->col, grid_sq->row);
                        }

                        if (!is_king_in_danger(player_white, test_state))
                        {
                            // if space is blank OR has opponent piece
                            if (grid_sq->piece.type == Piece_None || grid_sq->piece.white != held_piece.white)
                            {
                                // get rid of opponent piece
                                if (grid_sq->piece_sprite != 0)
                                {
                                    sprite_delete(grid_sq->piece_sprite);
                                }
                                
                                if (held_last_col != grid_sq->col || held_last_row != grid_sq->row)
                                {
                                    new_turn = true;
                                    
                                    if (held_last_col < 0 || held_last_row < 0)
                                    {
                                        // this came from panel, update count
                                        PieceButton* btn = &piece_buttons[held_piece.type - 1];
                                        --btn->count[player_white];

                                        // skip updating text cause we're about to switch to next player's turn anyway
                                    }
                                }

                                grid_sq->piece = held_piece;
                                grid_sq->piece_sprite = held_sprite;

                                if (turn_move)
                                {
                                    grid[held_last_col][held_last_row].piece.type = Piece_None;
                                }

                                sprite_set_pos(held_sprite, grid_pos_x + grid_sq->col * 14, grid_pos_y + grid_sq->row * 14);
                                sprite_set_layer(held_sprite, Layer_PlacedPiece);
                                held_piece.type = Piece_None;
                                held_sprite = 0;

                                check_for_check(true);
                                check_for_check(false);

                                held_last_col = -1;
                                held_last_row = -1;
                            }

                            click_used = true;
                        }
                    }
                    // pick up from square
                    else if (g_mouse_down && turn_move && grid_sq->piece.white == player_white && held_piece.type == Piece_None && grid_sq->piece.type != Piece_None)
                    {
                        set_all_valid_moves(grid_sq->col, grid_sq->row);

                        held_piece = grid_sq->piece;
                        held_sprite = grid_sq->piece_sprite;
                        sprite_set_layer(held_sprite, Layer_HeldPiece);

                        held_last_col = grid_sq->col;
                        held_last_row = grid_sq->row;

                        grid_sq->piece_sprite = 0;

                        click_used = true;
                    }

                    if (held_piece.type != Piece_None)
                    {
                        // hovering over origin
                        if (grid_sq->col == held_last_col && grid_sq->row == held_last_row)
                        {
                            show_highlights = true;

                            // newly hovering over this space
                            if (!(last_hover_col == grid_sq->col && last_hover_row == grid_sq->row))
                            {
                                // highlight all valid spaces
                                for (uint16 row = 0; row < 8; ++row)
                                {
                                    for (uint16 col = 0; col < 8; ++col)
                                    {
                                        highlight_square(col, row, is_valid_space(col, row));
                                    }
                                }
                            }
                        }
                        // hovering over valid space
                        else if (is_valid_space(grid_sq->col, grid_sq->row))
                        {
                            hovering = true;
                            show_preview = true;
                            // show piece preview
                            if (preview_sprite == 0)
                                preview_sprite = sprite_create(piece_textures[held_piece.type - 1], grid_pos_x + grid_sq->col * 14, grid_pos_y + grid_sq->row * 14, Layer_PlacedPiece, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f);
                            else
                                sprite_set_pos(preview_sprite, grid_pos_x + grid_sq->col * 14, grid_pos_y + grid_sq->row * 14);

                            if (turn_move)
                            {
                                show_highlights = true;

                                // newly hovering over this space
                                if (!(last_hover_col == grid_sq->col && last_hover_row == grid_sq->row))
                                {
                                    clear_overlays();

                                    if (held_piece.type == Piece_Pawn || held_piece.type == Piece_King || held_piece.type == Piece_Knight)
                                    {
                                        highlight_square(grid_sq->col, grid_sq->row, true);
                                    }
                                    else
                                    {
                                        int16 col_it = grid_sq->col - held_last_col;
                                        if (col_it != 0)
                                            col_it = SDL_abs(col_it) / col_it;

                                        int16 row_it = grid_sq->row - held_last_row;
                                        if (row_it != 0)
                                            row_it = SDL_abs(row_it) / row_it;

                                        uint16 col = held_last_col;
                                        uint16 row = held_last_row;
                                        while (col != grid_sq->col || row != grid_sq->row)
                                        {
                                            col += col_it;
                                            row += row_it;

                                            highlight_square(col, row, true);
                                        }
                                    }
                                }
                            }
                        }

                        last_hover_col = grid_sq->col;
                        last_hover_row = grid_sq->row;
                    }
                    else
                    {
                        if (turn_move && grid_sq->piece.white == player_white)
                        {
                            hovering = true;
                        }

                        last_hover_col = 9;
                        last_hover_row = 9;
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
    }

    if (!show_preview)
    {
        if (preview_sprite != 0)
        {
            sprite_delete(preview_sprite);
            preview_sprite = 0;
        }
    }

    if (!show_highlights)
    {
        clear_overlays();
    }

    if (!click_used && (g_mouse_down || g_mouse_long_up))
    {
        if (held_piece.type != Piece_None)
        {
            if (held_last_col >= 0 && held_last_row >= 0)
            {
                grid[held_last_col][held_last_row].piece_sprite = held_sprite;
                sprite_set_pos(held_sprite, grid_pos_x + held_last_col * 14, grid_pos_y + held_last_row * 14);
            }
            else
            {
                if (held_sprite != 0)
                    sprite_delete(held_sprite);
            }

            held_piece.type = Piece_None;
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
