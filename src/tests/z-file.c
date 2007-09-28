/*
 * Simple test rig for z-file functions
 * Compile and link with z-virt, z-util, z-form, and z-file.
 *
 * (c) Andrew Sidwell, 2007
 *
 * Permission is granted free of charge to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicence, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 */
#include <stdio.h>
#include "../z-file.h"
#include "../z-tests.h"
/*** z-file tests ***/

static const char *test_fname = "./test";

TEST(file_open)
{
	ang_file *f;

	/* Try opening a new file */
	f = file_open(test_fname, MODE_WRITE, FTYPE_TEXT);
	if (!f) FAIL;

	/* Try closing that file */
	if (!file_close(f)) FAIL;
}
TEST_END

TEST(file_exists)
{
	/* Test for the existence of the file "test" */
	if (!file_exists(test_fname)) FAIL;
}
TEST_END

TEST(file_lock)
{
	/* Not sure what to do here -- spawn off another process? */
	void file_lock(ang_file *f);
	void file_unlock(ang_file *f);
}
TEST_END

TEST(file_rws)
{
	ang_file *f;
	char buffer[1024];
	char comparison_buffer[1024];
	size_t i;
	u32b seek_positions[] = { 5, 18, 500, 95, 1023 };
	bool success;

	/* Set up buffer */
	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = (char) i * 3;

	printf("opening, ");
	f = file_open(test_fname, MODE_WRITE, FTYPE_TEXT);
	if (!f) FAIL;

	printf("writing, ");
	success = file_write(f, buffer, sizeof(buffer));
	file_close(f);
	if (!success) FAIL;

	printf("reopening, ");
	f = file_open(test_fname, MODE_READ, -1);
	if (!f) FAIL;

	/* Make sure the contents match up */	
	printf("reading, ");
	file_read(f, comparison_buffer, sizeof(buffer));
	for (i = 0; i < sizeof(buffer); i++)
	{
		if (buffer[i] != comparison_buffer[i])
			FAIL;
	}

	/* Seek, and make sure they still match up */
	printf("seeking ");
	for (i = 0; i < N_ELEMENTS(seek_positions); i++)
	{
		u32b pos = seek_positions[i];
		byte b;

		printf("%d", i);
		if (!file_seek(f, pos))
		{
			file_close(f);
			FAIL;
		}

		printf("A");
		if (!file_readc(f, &b))
		{
			file_close(f);
			FAIL;
		}

		printf("B ");
		if ((char)b != buffer[pos])
		{
			printf("was %d, should be %d", (char)b, (char)buffer[pos]);
			file_close(f);
			FAIL;
		}
	}	
}
TEST_END

TEST(file_chario)
{
	size_t i;
	ang_file *f;

	/* Try opening a new file */
	printf("opening, ");
	f = file_open(test_fname, MODE_WRITE, FTYPE_TEXT);
	if (!f) FAIL;

	for (i = 0; i < 25; i++)
	{
		if (!file_writec(f, (byte)i))
		{
			file_close(f);
			FAIL;
		}
	}
	file_close(f);
	
	/* Re-open for reading */
	printf("reopening, ");
	f = file_open(test_fname, MODE_READ, FTYPE_TEXT);
	if (!f) FAIL;

	for (i = 0; i < 25; i++)
	{
		byte ch;

		if (!file_readc(f, &ch))
		{
			file_close(f);
			FAIL;
		}

		if ((char)ch != i)
		{
			file_close(f);
			FAIL;
		}
	}
}
TEST_END

TEST(file_lineio)
{
	ang_file *f;
	size_t i;
	const char *lineendings[] = { "\n", "\r", "\r\n" };
	const char *text[] =
	{
		"'Twas brillig, and the slithy toves",
		"  Did gyre and gimble in the wabe",
		"All mimsy were the borogoves",
		"  And the mome raths outgrabe",
		"",
		"Beware the Jabberwock, my son!",
		"  The jaws that bite, the claws that catch!",
		"Beware the Jubjub bird, and shun",
		"  The frumious Bandersnatch!",
	};

	/* Try opening a new file */
	printf("opening, ");
	f = file_open(test_fname, MODE_WRITE, FTYPE_TEXT);
	if (!f) FAIL;

	/* Put the text, line by line */
	printf("writing, ");
	for (i = 0; i < N_ELEMENTS(text); i++)
	{
		if (!file_put(f, text[i]))
		{
			file_close(f);
			FAIL;
		}

		if (!file_put(f, lineendings[i % N_ELEMENTS(lineendings)]))
		{
			file_close(f);
			FAIL;
		}
	}
	file_close(f);
	
	/* Re-open for reading */
	printf("reopening, ");
	f = file_open(test_fname, MODE_READ, FTYPE_TEXT);
	if (!f) FAIL;

	printf("reading, ");
	for (i = 0; i < N_ELEMENTS(text); i++)
	{
		char buf[1024];
		printf("%d", i);
		if (!file_getl(f, buf, sizeof(buf)))
		{
			file_close(f);
			FAIL;
		}
		printf("B ");
		if (strcmp(buf, text[i]))
		{
			printf("%s %s", buf, text[i]);
			file_close(f);
			FAIL;
		}
	}

	file_close(f);
}
TEST_END

TEST(file_move)
{
	printf("move, ");
	if (!file_move(test_fname, "test2")) FAIL;
	printf("check, ");
	if (!file_exists("test2")) FAIL;
	printf("move, ");
	if (!file_move("test2", test_fname)) FAIL;
	printf("check, ");
	if (!file_exists(test_fname)) FAIL;
}
TEST_END

TEST(file_newer)
{
	if (!file_exists("z-file.c"))
	{
		printf("need z-file.c ");
		FAIL;
	}

	if (!file_exists(test_fname))
	{
		printf("need %s ", test_fname);
		FAIL;
	}

	printf("testing existent files 1 ");
	if (file_newer("z-file.c", test_fname)) FAIL;
	printf("2 ");
	if (!file_newer(test_fname, "z-file.c")) FAIL;

	printf("testing nonexistent files 1 ");
	if (file_newer("nonexistent", "z-file.c")) FAIL;
	printf("2 ");
	if (!file_newer("z-file.c", "nonexistent")) FAIL;
}
TEST_END

TEST(file_delete)
{
	if (!file_delete(test_fname)) FAIL;
	if (file_exists(test_fname)) FAIL;
}
TEST_END


TEST(path_build)
{
	char buf[1024];

	path_build(buf, sizeof(buf), "some" PATH_SEP "path", "extra");
	if (strcmp(buf, "some" PATH_SEP "path" PATH_SEP "extra")) FAIL;

	path_build(buf, sizeof(buf), "", "leaf");
	if (strcmp(buf, "leaf")) FAIL;

	path_build(buf, sizeof(buf), "base", "");
	if (strcmp(buf, "base")) FAIL;

	path_build(buf, sizeof(buf), "somepath", PATH_SEP "absolutepath");
	if (strcmp(buf, PATH_SEP "absolutepath")) FAIL;

#ifdef SET_UID
	path_build(buf, sizeof(buf), "~", "");
	if (!strcmp(buf, "~")) FAIL;

	path_build(buf, sizeof(buf), "~" PATH_SEP "dull", "");
	if (!strcmp(buf, "~/dull")) FAIL;
	if (!suffix(buf, PATH_SEP "dull")) FAIL;

	path_build(buf, sizeof(buf), "~ajps", "");
	if (!strcmp(buf, "~ajps")) FAIL;

	path_build(buf, sizeof(buf), "somepath", "~");
	if (!strcmp(buf, "somepath")) FAIL;
#endif
}
TEST_END


int main(void)
{
	TEST_PLAN(test, 10);

	TEST_RUN(&test, file_open);
	TEST_RUN(&test, file_exists);
	TEST_RUN(&test, file_lock);
	TEST_RUN(&test, file_rws);
	TEST_RUN(&test, file_chario);
	TEST_RUN(&test, file_lineio);
	TEST_RUN(&test, file_move);
	TEST_RUN(&test, file_newer);
	TEST_RUN(&test, file_delete);

	TEST_RUN(&test, path_build);

	TESTS_COMPLETE(test);
}
