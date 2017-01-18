/*
 * Avalon memory-mapped peripheral for the VGA LED Emulator
 *
 * Stephen A. Edwards
 * Columbia University
 *
 *
 * Final Project
 *
 * Name: Bernardo de Almeida Abreu
 * UNI:  bd2440
 *
 * Name: Henrique Pizzol Grando
 * UNI:  hp2409
 * 
 * Name: Lucas Ikenaga Barros
 * UNI:  li2176
 *
 * Name: Tomas Mantelato
 * UNI:  tm2779
 *
 */

module VGA_LED(input logic        clk,
	       input logic 	  reset,
	       input logic [31:0]  writedata,
	       input logic 	  write,
	       input 		  chipselect,
	       input logic address,

	       output logic [7:0] VGA_R, VGA_G, VGA_B,
	       output logic 	  VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n,
	       output logic 	  VGA_SYNC_n);

	 
	logic [9:0]  posx,posy;
	logic [3:0] s_shape;
	logic s_invert;
	logic [2:0] s_count;
	logic [2:0] id;
	logic layer;
	logic [3:0] life_1, life_2;
	logic [7:0] bR,bG,bB;

   VGA_LED_Emulator led_emulator(.clk50(clk), .*);

   always_ff @(posedge clk)
     if (reset) begin
			posx <= 10'b0010000000;
			posy <= 10'b0010000000;
			s_invert <= 1'd0;
			s_count <= 3'd0;
			id <= 3'd0;
			s_shape <= 4'd0;
			layer <= 1'd0;
			life_1 <= 4'd0;
			life_2 <= 4'd0;
			bR <= 8'h35;
			bG <= 8'hbc;
			bB <= 8'hff;
     end else if (chipselect && write)
       case (address)
			1'h0 :
			begin	
				posx <= writedata[9:0];
				posy <= writedata[19:10];
				s_invert <= writedata[20];
				s_count <= writedata[23:21];
				id <= writedata[26:24];
				s_shape <= writedata[30:27];
				layer <= writedata[31];
			end
			1'h1 :
			begin
				life_1 <= writedata[3:0];
				life_2 <= writedata[7:4];
				bB <= writedata[15:8];
				bG <= writedata[23:16];
				bR <= writedata[31:24];
			end
		endcase
	       
endmodule
