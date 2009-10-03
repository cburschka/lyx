#!/bin/bash

# Cache-bisect is
# roughly based on git-bisect, but uses SVN and caches every build tree
# to maximize performance. The idea here is that if the first few tests
# happen very quickly, and give plausible results, you can have
# confidence that you can just leave it running. Without that confidence
# I'd be continually thinking "Is the bisect still working, maybe I
# should check" so the bisect would be taking up my mental space. With
# cache-bisect, I can fire-and-forget, leaving the CPU to do the menial
# tasks while I think about other things. Additionally caching the
# build-trees cuts down on the amount of bandwidth required on the SVN
# server
#  It uses three levels
#    1) At the bottom level, it bisects much like git-bisect
#    2) At the second level, it bisects, but only over cached revisions.
#    3) The the top level it does what I refer to as a "multisect". The
# idea is that you may not know which revision was a good revision, and
# picking revision 1 as the "good" revision is not only mildly wasteful
# but is likely to produce misleading results. Instead it starts
# stepping backwards, first trying 1.6.0, and then going backwards
# through trunk.

	KT=$(cd $(dirname "$0") && pwd)
$KT/cache-bisect.py "$@" | tee out/cache-bisect-$USER.log


