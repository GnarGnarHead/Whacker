#ifndef WHACKER_TEST_ASSERTIONS_ACTIVE
#error "WHACKER_TEST_ASSERTIONS_ACTIVE must be defined for smoke tests."
#endif

#ifdef NDEBUG
#error "NDEBUG must be undefined for smoke tests."
#endif

int main() {
    return 0;
}
