//
// Copyright (C) 2017  Peter Bartmann <borti4938@gmx.de>
//
// This file is part of Open Source Scan Converter project, which is
// Copyright (C) 2015-2017  Markus Hiienkari <mhiienka@niksula.hut.fi>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

module lcdbl_timeout(
    clk27,
    reset_n,
    lt_active,
    ir_in,
    ir_out,
    btn_in,
    btn_out,
    lcdbl_off,
    lcdbl_out
);

localparam     tocnt_w = 32;
parameter  tocnt_start = 32'd1215000000;  // 45s (number = timeout * 27000000)
                                          // max. 2^32-1 = 4294967295 (~159s)

input             clk27;
input             reset_n;
input             lt_active;
input      [23:0] ir_in;
output reg [23:0] ir_out = 24'd0;
input      [ 1:0] btn_in;
output reg [ 1:0] btn_out = 2'b11;
input             lcdbl_off;
output reg        lcdbl_out = 1'b1;

reg [tocnt_w-1:0] timeout_cnt = tocnt_start;
reg pass_vals = 1'b0;

wire in_active = (ir_in[15:0] != 0) || (~&btn_in);

reg init_phase     = 1'b1;
reg lcdbl_off_L    = 1'b0;
reg turn_lcdbl_off = 1'b0;

wire trigger_lcdbl_on  = in_active || !reset_n;
wire trigger_lcdbl_off = !init_phase && (lcdbl_off_L ^ lcdbl_off);

always @(posedge clk27)
begin

    if (|timeout_cnt) begin
        if (trigger_lcdbl_on)
            timeout_cnt <= tocnt_start;
        else
            timeout_cnt <= timeout_cnt - 1;

        if (pass_vals) begin
             ir_out[15:0] <=  ir_in[15:0];
            btn_out       <= btn_in;
        end

        if (!in_active) begin
            pass_vals <= 1'b1;
        end

        if (trigger_lcdbl_on)
            timeout_cnt <= tocnt_start;
        lcdbl_out   <= 1'b1;

        if (trigger_lcdbl_off) begin
            timeout_cnt    <= {tocnt_w{1'b0}};
            turn_lcdbl_off <= 1'b1;
              ir_out[15:0] <= 16'h0;
             btn_out       <= 2'b11;
        end

        lcdbl_out <= 1'b1;
    end else begin
        pass_vals     <= 1'b0;
         ir_out[15:0] <= 16'h0;
        btn_out       <= 2'b11;
        lcdbl_out     <= 1'b0;

        init_phase    <= !reset_n;

        if (!turn_lcdbl_off && trigger_lcdbl_on)
            timeout_cnt <= tocnt_start;
        if (!in_active)
            turn_lcdbl_off <= 1'b0;
    end

    ir_out[23:16] <= ir_in[23:16];
    lcdbl_off_L   <= lcdbl_off;
end

endmodule
