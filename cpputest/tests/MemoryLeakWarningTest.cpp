/*
 * Copyright (c) 2007, Michael Feathers, James Grenning and Bas Vodde
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE EARLIER MENTIONED AUTHORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"
#include "CppUTest/TestOutput.h"
#include "CppUTest/MemoryLeakWarningPlugin.h"
#include "CppUTest/MemoryLeakDetector.h"
#include "CppUTest/TestTestingFixture.h"

static char* arrayToLeak1;
static char* arrayToLeak2;
static long* nonArrayToLeak;

class DummyReporter : public MemoryLeakFailure
{
public:
   virtual ~DummyReporter() {};
   virtual void fail(char* fail_string) {};
};

static MemoryLeakDetector* detector;
static MemoryLeakWarningPlugin* memPlugin;
static DummyReporter dummy;


TEST_GROUP(MemoryLeakWarningTest)
{
	TestTestingFixture* fixture;
	void setup()
	{
		fixture = new TestTestingFixture();
		detector = new MemoryLeakDetector();
		detector->init(&dummy);
  		memPlugin = new MemoryLeakWarningPlugin("TestMemoryLeakWarningPlugin", detector);
  		fixture->registry->installPlugin(memPlugin);
  		memPlugin->enable();

		arrayToLeak1 = 0;
		arrayToLeak2 = 0;
		nonArrayToLeak = 0;
	}
	void teardown()
	{
      detector->freeOperatorDeleteArray(arrayToLeak1);
      detector->freeOperatorDeleteArray(arrayToLeak2);
      detector->freeOperatorDelete((char*)nonArrayToLeak);

      delete fixture;
  		delete memPlugin;
  		delete detector;
	}
};

void _testExpectOneArrayLeak()
{
   memPlugin->expectLeaksInTest(1);
   arrayToLeak1 = detector->allocOperatorNewArray(100);
}

TEST(MemoryLeakWarningTest, Ignore1)
{
	fixture->setTestFunction(_testExpectOneArrayLeak);
	fixture->runAllTests();
	LONGS_EQUAL(0, fixture->getFailureCount());
}

void _testOneArrayLeak()
{
   arrayToLeak1 = detector->allocOperatorNewArray(100);
}

TEST(MemoryLeakWarningTest, OneArrayLeak)
{
   fixture->setTestFunction(_testOneArrayLeak);
   fixture->runAllTests();
   LONGS_EQUAL(1, fixture->getFailureCount());
   fixture->assertPrintContains("Total number of leaks:  1");
}

void _testTwoLeaks()
{
    arrayToLeak2 = detector->allocOperatorNewArray(10);;
    nonArrayToLeak = (long*) detector->allocOperatorNew(4);
}

TEST(MemoryLeakWarningTest, TwoLeaks)
{
	fixture->setTestFunction(_testTwoLeaks);
	fixture->runAllTests();
   LONGS_EQUAL(1, fixture->getFailureCount());
   fixture->assertPrintContains("Total number of leaks:  2");
}

void _testIgnore2()
{
   memPlugin->expectLeaksInTest(2);
   arrayToLeak2 = detector->allocOperatorNewArray(10);
   nonArrayToLeak = (long*) detector->allocOperatorNew(4);
}

TEST(MemoryLeakWarningTest, Ignore2)
{
	fixture->setTestFunction(_testIgnore2);
	fixture->runAllTests();
	LONGS_EQUAL(0, fixture->getFailureCount());
}

static void _failAndLeakMemory()
{
   arrayToLeak2 = detector->allocOperatorNewArray(10);
   FAIL("");
}

TEST(MemoryLeakWarningTest, FailingTestDoesNotReportMemoryLeaks)
{
   fixture->setTestFunction(_failAndLeakMemory);
   fixture->runAllTests();
   LONGS_EQUAL(1, fixture->getFailureCount());
}
