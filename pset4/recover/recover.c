#include <stdio.h>
#include <stdint.h>

typedef uint8_t BYTE;

int is_jpeg_signature(BYTE bytes[]);

int main(int argc, char *argv[])
{
    // Make sure user didn't entered more than one arg
    if (argc != 2)
    {
        printf("Usage: ./recover image\n");
    }

    // Assume second arg is filename and open file to read
    char *cardFilename = argv[1];
    FILE *card = fopen(cardFilename, "r");

    // If supplied arg can't be opened as a file, inform user and return 1
    if (card == NULL)
    {
        printf("File can't be opened.\n");
        return 1;
    }

    // JPEG signature placeholder
    BYTE bytes[4];

    // Keep track of which file we're recovering
    int i = 0;

    // Read 4 bytes from cursor position
    while (fread(&bytes, 1, 4, card) != 0)
    {
        if (is_jpeg_signature(bytes))
        {
            // Place the cursor back to begining of image
            fseek(card, -4, SEEK_CUR);

            // Create filename for current image
            char imageFilename[8];
            sprintf(imageFilename, "%03i.jpg", i);

            // Open file to write current image
            FILE *image = fopen(imageFilename, "w");

            // JPEG chunk placeholder
            BYTE buffer[512];

            // Keep track of first buffer read
            int is_subsequent_read = 0;

            // Read 1 chunk of 512 bytes at a time
            // until end of file is reached
            while (fread(&buffer, 1, 512, card) != 0)
            {
                // If next JPEG signature found
                if (is_subsequent_read && is_jpeg_signature(buffer))
                {
                    // Rewind cursor to start of image
                    fseek(card, -512, SEEK_CUR);

                    // Continue to outer loop and close file
                    break;
                }

                // Write buffer to image
                fwrite(buffer, 1, 512, image);
                is_subsequent_read = 1;
            }

            // Close image file
            fclose(image);

            // Increment image counter
            i++;
        }
        else
        {
            // If current 4 bytes is not a JPEG signature
            // replace cursor to second byte and search for
            // following 4 bytes and so on
            fseek(card, -3, SEEK_CUR);
        }

    }

    // Close card file
    fclose(card);
}


int is_jpeg_signature(BYTE bytes[])
{
    return bytes[0] == 0xff
        && bytes[1] == 0xd8
        && bytes[2] == 0xff
        && bytes[3] >= 0xe0
        && bytes[3] <= 0xef;
}
