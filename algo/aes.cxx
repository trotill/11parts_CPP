/*
 * aes.cxx
 *
 *  Created on: 18 окт. 2019 г.
 *      Author: root
 */


#include "aes.h"

const unsigned int BLOCK_BYTES_LENGTH = 16 * sizeof(unsigned char);

AES::AES(int keyLen)
{
  this->Nb = 4;
  switch (keyLen)
  {
  case 128:
    this->Nk = 4;
    this->Nr = 10;
    break;
  case 192:
    this->Nk = 6;
    this->Nr = 12;
    break;
  case 256:
    this->Nk = 8;
    this->Nr = 14;
    break;
  default:
    throw "Incorrect key length";
  }

  blockBytesLen = 4 * this->Nb * sizeof(unsigned char);
}

unsigned char * AES::EncryptECB(unsigned char in[], unsigned int inLen, unsigned  char key[], unsigned int &outLen)
{
  outLen = GetPaddingLength(inLen);
  unsigned char *alignIn  = PaddingNulls(in, inLen, outLen);
  unsigned char *out = new unsigned char[outLen];
  for (unsigned int i = 0; i < outLen; i+= blockBytesLen)
  {
    EncryptBlock(alignIn + i, out + i, key);
  }

  delete[] alignIn;

  return out;
}

unsigned char * AES::DecryptECB(unsigned char in[], unsigned int inLen, unsigned  char key[])
{
  unsigned char *out = new unsigned char[inLen];
  for (unsigned int i = 0; i < inLen; i+= blockBytesLen)
  {
    DecryptBlock(in + i, out + i, key);
  }

  return out;
}


unsigned char *AES::EncryptCBC(unsigned char in[], unsigned int inLen, unsigned  char key[], unsigned char * iv, unsigned int &outLen)
{
  outLen = GetPaddingLength(inLen);
  unsigned char *alignIn  = PaddingNulls(in, inLen, outLen);
  unsigned char *out = new unsigned char[outLen];
  unsigned char *block = new unsigned char[blockBytesLen];
  memcpy(block, iv, blockBytesLen);
  for (unsigned int i = 0; i < outLen; i+= blockBytesLen)
  {
    XorBlocks(block, alignIn + i, block, blockBytesLen);
    EncryptBlock(block, out + i, key);
    memcpy(block, out + i, blockBytesLen);
  }

  delete[] block;
  delete[] alignIn;

  return out;
}

unsigned char *AES::DecryptCBC(unsigned char in[], unsigned int inLen, unsigned  char key[], unsigned char * iv)
{
  unsigned char *out = new unsigned char[inLen];
  unsigned char *block = new unsigned char[blockBytesLen];
  memcpy(block, iv, blockBytesLen);
  for (unsigned int i = 0; i < inLen; i+= blockBytesLen)
  {
    DecryptBlock(in + i, out + i, key);
    XorBlocks(block, out + i, out + i, blockBytesLen);
    memcpy(block, in + i, blockBytesLen);
  }

  delete[] block;

  return out;
}

unsigned char *AES::EncryptCFB(unsigned char in[], unsigned int inLen, unsigned  char key[], unsigned char * iv, unsigned int &outLen)
{
  outLen = GetPaddingLength(inLen);
  unsigned char *alignIn  = PaddingNulls(in, inLen, outLen);
  unsigned char *out = new unsigned char[outLen];
  unsigned char *block = new unsigned char[blockBytesLen];
  unsigned char *encryptedBlock = new unsigned char[blockBytesLen];
  memcpy(block, iv, blockBytesLen);
  for (unsigned int i = 0; i < outLen; i+= blockBytesLen)
  {
    EncryptBlock(block, encryptedBlock, key);
    XorBlocks(alignIn + i, encryptedBlock, out + i, blockBytesLen);
    memcpy(block, out + i, blockBytesLen);
  }

  delete[] block;
  delete[] encryptedBlock;
  delete[] alignIn;

  return out;
}

unsigned char *AES::DecryptCFB(unsigned char in[], unsigned int inLen, unsigned  char key[], unsigned char * iv)
{
  unsigned char *out = new unsigned char[inLen];
  unsigned char *block = new unsigned char[blockBytesLen];
  unsigned char *encryptedBlock = new unsigned char[blockBytesLen];
  memcpy(block, iv, blockBytesLen);
  for (unsigned int i = 0; i < inLen; i+= blockBytesLen)
  {
    EncryptBlock(block, encryptedBlock, key);
    XorBlocks(in + i, encryptedBlock, out + i, blockBytesLen);
    memcpy(block, in + i, blockBytesLen);
  }

  delete[] block;
  delete[] encryptedBlock;

  return out;
}

unsigned char * AES::PaddingNulls(unsigned char in[], unsigned int inLen, unsigned int alignLen)
{
  unsigned char *alignIn = new unsigned char[alignLen];
  memcpy(alignIn, in, inLen);
  memset(alignIn + inLen, 0x00, alignLen - inLen);
  return alignIn;
}

unsigned int AES::GetPaddingLength(unsigned int len)
{
  unsigned int lengthWithPadding =  (len / blockBytesLen);
  if (len % blockBytesLen) {
	  lengthWithPadding++;
  }

  lengthWithPadding *=  blockBytesLen;

  return lengthWithPadding;
}

void AES::EncryptBlock(unsigned char in[], unsigned char out[], unsigned  char key[])
{
  unsigned char *w = new unsigned char[4 * Nb * (Nr + 1)];
  KeyExpansion(key, w);
  unsigned char **state = new unsigned char *[4];
  state[0] = new unsigned  char[4 * Nb];
  int i, j, round;
  for (i = 0; i < 4; i++)
  {
    state[i] = state[0] + Nb * i;
  }


  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < Nb; j++)
    {
      state[i][j] = in[i + 4 * j];
    }
  }

  AddRoundKey(state, w);

  for (round = 1; round <= Nr - 1; round++)
  {
    SubBytes(state);
    ShiftRows(state);
    MixColumns(state);
    AddRoundKey(state, w + round * 4 * Nb);
  }

  SubBytes(state);
  ShiftRows(state);
  AddRoundKey(state, w + Nr * 4 * Nb);

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < Nb; j++)
    {
      out[i + 4 * j] = state[i][j];
    }
  }

  delete[] state[0];
  delete[] state;
  delete[] w;
}

void AES::DecryptBlock(unsigned char in[], unsigned char out[], unsigned  char key[])
{
  unsigned char *w = new unsigned char[4 * Nb * (Nr + 1)];
  KeyExpansion(key, w);
  unsigned char **state = new unsigned char *[4];
  state[0] = new unsigned  char[4 * Nb];
  int i, j, round;
  for (i = 0; i < 4; i++)
  {
    state[i] = state[0] + Nb * i;
  }


  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < Nb; j++) {
      state[i][j] = in[i + 4 * j];
    }
  }

  AddRoundKey(state, w + Nr * 4 * Nb);

  for (round = Nr - 1; round >= 1; round--)
  {
    InvSubBytes(state);
    InvShiftRows(state);
    AddRoundKey(state, w + round * 4 * Nb);
    InvMixColumns(state);
  }

  InvSubBytes(state);
  InvShiftRows(state);
  AddRoundKey(state, w);

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < Nb; j++) {
      out[i + 4 * j] = state[i][j];
    }
  }

  delete[] state[0];
  delete[] state;
  delete[] w;
}


void AES::SubBytes(unsigned char **state)
{
  int i, j;
  unsigned char t;
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < Nb; j++)
    {
      t = state[i][j];
      state[i][j] = sbox[t / 16][t % 16];
    }
  }

}

void AES::ShiftRow(unsigned char **state, int i, int n)    // shift row i on n positions
{
  unsigned char t;
  int k, j;
  for (k = 0; k < n; k++)
  {
    t = state[i][0];
    for (j = 0; j < Nb - 1; j++)
    {
      state[i][j] = state[i][j + 1];
    }
    state[i][Nb - 1] = t;
  }
}

void AES::ShiftRows(unsigned char **state)
{
  ShiftRow(state, 1, 1);
  ShiftRow(state, 2, 2);
  ShiftRow(state, 3, 3);
}

unsigned char AES::xtime(unsigned char b)    // multiply on x
{
  unsigned char mask = 0x80, m = 0x1b;
  unsigned char high_bit = b & mask;
  b = b << 1;
  if (high_bit) {    // mod m(x)
    b = b ^ m;
  }
  return b;
}

unsigned char AES::mul_bytes(unsigned char a, unsigned char b)
{
  unsigned char c = 0, mask = 1, bit, d;
  int i, j;
  for (i = 0; i < 8; i++)
  {
    bit = b & mask;
    if (bit)
    {
      d = a;
      for (j = 0; j < i; j++)
      {    // multiply on x^i
        d = xtime(d);
      }
      c = c ^ d;    // xor to result
    }
    b = b >> 1;
  }
  return c;
}

void AES::MixColumns(unsigned char **state)
{
  unsigned char s[4], s1[4];
  int i, j;

  for (j = 0; j < Nb; j++)
  {
    for (i = 0; i < 4; i++)
    {
      s[i] = state[i][j];
    }

    s1[0] = mul_bytes(0x02, s[0]) ^ mul_bytes(0x03, s[1]) ^ s[2] ^ s[3];
    s1[1] = s[0] ^ mul_bytes(0x02, s[1]) ^ mul_bytes(0x03, s[2]) ^ s[3];
    s1[2] = s[0] ^ s[1] ^ mul_bytes(0x02, s[2]) ^ mul_bytes(0x03, s[3]);
    s1[3] = mul_bytes(0x03, s[0]) ^ s[1] ^ s[2] ^ mul_bytes(0x02, s[3]);
    for (i = 0; i < 4; i++)
    {
      state[i][j] = s1[i];
    }

  }

}

void AES::AddRoundKey(unsigned char **state, unsigned char *key)
{
  int i, j;
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < Nb; j++)
    {
      state[i][j] = state[i][j] ^ key[i + 4 * j];
    }
  }
}

void AES::SubWord(unsigned char *a)
{
  int i;
  for (i = 0; i < 4; i++)
  {
    a[i] = sbox[a[i] / 16][a[i] % 16];
  }
}

void AES::RotWord(unsigned char *a)
{
  unsigned char c = a[0];
  a[0] = a[1];
  a[1] = a[2];
  a[2] = a[3];
  a[3] = c;
}

void AES::XorWords(unsigned char *a, unsigned char *b, unsigned char *c)
{
  int i;
  for (i = 0; i < 4; i++)
  {
    c[i] = a[i] ^ b[i];
  }
}

void AES::Rcon(unsigned char * a, int n)
{
  int i;
  unsigned char c = 1;
  for (i = 0; i < n - 1; i++)
  {
    c = xtime(c);
  }

  a[0] = c;
  a[1] = a[2] = a[3] = 0;
}

void AES::KeyExpansion(unsigned char key[], unsigned char w[])
{
  unsigned char *temp = new unsigned char[4];
  unsigned char *rcon = new unsigned char[4];

  int i = 0;
  while (i < 4 * Nk)
  {
    w[i] = key[i];
    i++;
  }

  i = 4 * Nk;
  while (i < 4 * Nb * (Nr + 1))
  {
    temp[0] = w[i - 4 + 0];
    temp[1] = w[i - 4 + 1];
    temp[2] = w[i - 4 + 2];
    temp[3] = w[i - 4 + 3];

    if (i / 4 % Nk == 0)
    {
        RotWord(temp);
        SubWord(temp);
        Rcon(rcon, i / (Nk * 4));
      XorWords(temp, rcon, temp);
    }
    else if (Nk > 6 && i / 4 % Nk == 4)
    {
      SubWord(temp);
    }

    w[i + 0] = w[i - 4 * Nk] ^ temp[0];
    w[i + 1] = w[i + 1 - 4 * Nk] ^ temp[1];
    w[i + 2] = w[i + 2 - 4 * Nk] ^ temp[2];
    w[i + 3] = w[i + 3 - 4 * Nk] ^ temp[3];
    i += 4;
  }

  delete []rcon;
  delete []temp;

}


void AES::InvSubBytes(unsigned char **state)
{
  int i, j;
  unsigned char t;
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < Nb; j++)
    {
      t = state[i][j];
      state[i][j] = inv_sbox[t / 16][t % 16];
    }
  }
}

void AES::InvMixColumns(unsigned char **state)
{
  unsigned char s[4], s1[4];
  int i, j;

  for (j = 0; j < Nb; j++)
  {
    for (i = 0; i < 4; i++)
    {
      s[i] = state[i][j];
    }
    s1[0] = mul_bytes(0x0e, s[0]) ^ mul_bytes(0x0b, s[1]) ^ mul_bytes(0x0d, s[2]) ^ mul_bytes(0x09, s[3]);
    s1[1] = mul_bytes(0x09, s[0]) ^ mul_bytes(0x0e, s[1]) ^ mul_bytes(0x0b, s[2]) ^ mul_bytes(0x0d, s[3]);
    s1[2] = mul_bytes(0x0d, s[0]) ^ mul_bytes(0x09, s[1]) ^ mul_bytes(0x0e, s[2]) ^ mul_bytes(0x0b, s[3]);
    s1[3] = mul_bytes(0x0b, s[0]) ^ mul_bytes(0x0d, s[1]) ^ mul_bytes(0x09, s[2]) ^ mul_bytes(0x0e, s[3]);

    for (i = 0; i < 4; i++)
    {
      state[i][j] = s1[i];
    }
  }
}

void AES::InvShiftRows(unsigned char **state)
{
  ShiftRow(state, 1, Nb - 1);
  ShiftRow(state, 2, Nb - 2);
  ShiftRow(state, 3, Nb - 3);
}

void AES::XorBlocks(unsigned char *a, unsigned char * b, unsigned char *c, unsigned int len)
{
  for (unsigned int i = 0; i < len; i++)
  {
    c[i] = a[i] ^ b[i];
  }
}

void AES::printHexArray (unsigned char a[], unsigned int n)
{
	for (unsigned int i = 0; i < n; i++) {
	  printf("%02x ", a[i]);
	}
}

void example_OneBlockDecrypt(void )
{
  AES aes(128);
  unsigned char encrypted[] = { 0x7c, 0x99, 0xf4, 0x2b, 0x6e, 0xe5, 0x03, 0x30, 0x9c, 0x6c, 0x1a, 0x67, 0xe9, 0x7a, 0xc2, 0x42 };
  unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
  unsigned char right[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  unsigned int len = 0;
  unsigned char *out = aes.DecryptECB(encrypted, BLOCK_BYTES_LENGTH, key);

  memcmp(right, out, len);

  delete[] out;
}

void example_EncryptDecryptECB(void){
	AES aes(256);
	  unsigned char plain[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

	  unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
	    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
	  unsigned int len = 0;

	  unsigned char *out = aes.EncryptECB(plain, BLOCK_BYTES_LENGTH, key, len);
	  unsigned char *innew = aes.DecryptECB(out, BLOCK_BYTES_LENGTH, key);
	  memcmp(innew, plain, BLOCK_BYTES_LENGTH);
	  delete[] out;
	  delete[] innew;
}

void example_EncryptDecryptCBC(void){
	 AES aes(256);
	  unsigned char plain[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	  unsigned char iv[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	  unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
	    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
	  unsigned int len;

	  unsigned char *out = aes.EncryptCBC(plain, BLOCK_BYTES_LENGTH, key, iv, len);
	  unsigned char *innew = aes.DecryptCBC(out, BLOCK_BYTES_LENGTH, key, iv);
	  memcmp(innew, plain, BLOCK_BYTES_LENGTH);
	  delete[] out;
	  delete[] innew;
}

void example_EncryptDecryptCFB(void){
	 AES aes(256);
	  unsigned char plain[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	  unsigned char iv[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	  unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
	    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
	  unsigned int len;

	  unsigned char *out = aes.EncryptCFB(plain, BLOCK_BYTES_LENGTH, key, iv, len);
	  unsigned char *innew = aes.DecryptCFB(out, BLOCK_BYTES_LENGTH, key, iv);
	  memcmp(innew, plain, BLOCK_BYTES_LENGTH);
	  delete[] out;
	  delete[] innew;
}

void example_DecryptTwoBlocks(void)
{
  AES aes(128);
  unsigned char encrypted[] = {0x3c, 0x55, 0x3d, 0x01, 0x8a, 0x52, 0xe4, 0x54, 0xec, 0x4e, 0x08, 0x22, 0xc2, 0x8d, 0x55, 0xec,
                               0xe3, 0x5a, 0x40, 0xab, 0x30, 0x29, 0xf3, 0x0c, 0xe1, 0xdb, 0x30, 0x6c, 0xa1, 0x05, 0xcb, 0xa9};
  unsigned char iv[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
  unsigned char right[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
							0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

  unsigned char *out = aes.DecryptCFB(encrypted, BLOCK_BYTES_LENGTH * 2, key, iv);
  memcmp(right, out, BLOCK_BYTES_LENGTH * 2);
  delete[] out;
}

void example_EncryptTwoBlocks(void)
{
  AES aes(128);
  unsigned char plain[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
							0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
  unsigned char iv[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
  unsigned char right[] = {0x3c, 0x55, 0x3d, 0x01, 0x8a, 0x52, 0xe4, 0x54, 0xec, 0x4e, 0x08, 0x22, 0xc2, 0x8d, 0x55, 0xec,
                           0xe3, 0x5a, 0x40, 0xab, 0x30, 0x29, 0xf3, 0x0c, 0xe1, 0xdb, 0x30, 0x6c, 0xa1, 0x05, 0xcb, 0xa9};
  unsigned int len;

  unsigned char *out = aes.EncryptCFB(plain, BLOCK_BYTES_LENGTH * 2, key, iv, len);
  memcmp(right, out, BLOCK_BYTES_LENGTH * 2);
  delete[] out;
}
