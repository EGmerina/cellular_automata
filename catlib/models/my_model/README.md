![alt text](image.png)

int propogation(void *n)
{
    cellBody *cell = (cellBody *)n;
    int sum = -cell[0].compression;
    for (int i = 1; i <= 8; i++)
    {
        sum += cell[i].compression / 8;
    }
    cell[0].compression = sum;
    return 0;
}