#include <stdlib.h>
#include <glib.h>
#include "../utest/utest.inc"

#include "repr.h"
#include "repr-private.h"
#include "repr-action.h"

int main(int argc, char *argv[]) {
	SPReprDoc *document;
	SPRepr *a, *b, *c, *root;

	document = sp_repr_document_new("test");
	root = sp_repr_document_root(document);

	utest_start("XML Transactions");

	a = sp_repr_new("a");
	b = sp_repr_new("b");
	c = sp_repr_new("c");

	if (utest_test("rollback of node addition")) {
		sp_repr_begin_transaction(document);
		utest_assert(sp_repr_parent(a) == NULL);

		sp_repr_append_child(root, a);
		utest_assert(sp_repr_parent(a) == root);

		sp_repr_rollback(document);
		utest_assert(sp_repr_parent(a) == NULL);
	}

	if (utest_test("rollback of node removal")) {
		sp_repr_append_child(root, a);

		sp_repr_begin_transaction(document);
		utest_assert(sp_repr_parent(a) == root);

		sp_repr_unparent(a);
		utest_assert(sp_repr_parent(a) == NULL);

		sp_repr_rollback(document);
		utest_assert(sp_repr_parent(a) == root);
	}

	sp_repr_unparent(a);

	if (utest_test("rollback of node reordering")) {
		sp_repr_append_child(root, a);
		sp_repr_append_child(root, b);
		sp_repr_append_child(root, c);

		sp_repr_begin_transaction(document);
		utest_assert(sp_repr_next(a) == b);
		utest_assert(sp_repr_next(b) == c);
		utest_assert(sp_repr_next(c) == NULL);

		sp_repr_change_order(root, b, c);
		utest_assert(sp_repr_next(a) == c);
		utest_assert(sp_repr_next(b) == NULL);
		utest_assert(sp_repr_next(c) == b);

		sp_repr_rollback(document);
		utest_assert(sp_repr_next(a) == b);
		utest_assert(sp_repr_next(b) == c);
		utest_assert(sp_repr_next(c) == NULL);
	}

	sp_repr_unparent(a);
	sp_repr_unparent(b);
	sp_repr_unparent(c);

	/* lots more tests needed ... */

	return utest_end() ? 0 : 1;
}
