# Mastermind

Mastermind is a code-breaking game created in 1970 by Mordecai Meirowitz. The game will be recreated using an MSP430, a potentiometer, RGB LED, onboard button and 7-segment display.

## Output

```
> CMYK
? CMYK  P L
0 KBYY: 1 1
1 CMBR: 2 0
2 RGYK: 2 0
3 KMYC: 2 2
4 CMYK: 4 0 Won!
> RGBW
? RGBW  P L
0 RYBM: 2 0
1 MCBM: 1 0
2 MYRB: 0 2
3 CCMY: 0 0
4 MMWW: 1 0
5 WCMB: 0 2
6 YYYY: 0 0
7 YYYY: 0 0
8 YYYY: 0 0
9 YYYY: 0 0 Lost :(
>
```

##### NOTE: [`K` is Black](https://en.wikipedia.org/wiki/CMYK_color_model) (all lights off)

## Wiring

To use the `P2.6 (XIN)` and `P2.7 (XOUT)` pins as regular GPIO (general purpose input/output), you must clear the bits from the first special function select memory location:
```
P2SEL  &= ~(BIT6 | BIT7);
```

## Design

### Obtaining Keyboard Input

Initially the microcontroller should wait for input from the "mastermind" to choose the code to be broken (shown as a `>` prompt). The `MAX_LINE` defines how many character (minus one for the `\0` terminator!) should be allowed to be typed. Anything to be shown on the screen will be printed. Key presses are sent to the microcontroller, and the code will do something with it.

When the backspace key is pressed, the ASCII code `0x08` or `\b` is sent.

### Playing the Game

```
? CMYK  P L
```

* `?`: number of guess (starting at 0)
* `CMYK`: the code to break
* `P`: number of colors _that are in the correct position_
* `L`: number of correct colors _but in the wrong position_

Once the code is selected, the game then waits for the "player" to dial in the color with the potentiometer and enter that color by pressing the onboard button.

When four colors have been entered for the code, the game will check it against the answer and output:

1. correct colors in correct positions (black or colored pegs, `P` in the example output)
1. correct colors, but in incorrect positions (white pegs, `L` in the example output)

It will be shown on the 7-segment display to the "player". `P` on the left and `L` on the right.

If the "player" correctly guessed all 4 colors in the correct position then the game is won. If the "player" has tried to guess 10 times and still doesn't have the correct answer, then the game is lost.
