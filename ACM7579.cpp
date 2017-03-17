// ACM ICPC World Final Phuket 2016; Problem 7579 (Clock Breaking) 
// Author: Herbert Meiler (12/2016 - 01/2017)

#include <cassert>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>

struct Time
{
    Time() :
        Time(0, 0)
    {
    }

    Time(int hour, int minute) :
        hour(hour),
        minute(minute)
    {
        assert(hour >= 0);
        assert(hour < 24);
        assert(minute >= 0);
        assert(minute < 60);
    }

    Time(const Time&) = default;
    Time& operator = (const Time&) = default;

    operator std::int32_t() const
    {
        std::int32_t value = 0;

        auto p = reinterpret_cast<std::int8_t*>(&value);
        for (int index = 0; index != 4; ++index, ++p)
        {
            *p = Digit(index);
        }

        return value;
    }

    int Hour() const
    {
        return hour;
    }

    int Minute() const
    {
        return minute;
    }

    int Digits() const
    {
        return 4;
    }

    int Digit(int index) const
    {
        int digit;

        switch (index)
        {
        case 0:
            digit = minute % 10;
            break;
        case 1:
            digit = minute / 10;
            break;
        case 2:
            digit = hour % 10;
            break;
        case 3:
            digit = hour / 10;
            break;

        default:
            assert("invalid index");
            digit = -1;
        }

        return digit;
    }

    int operator[](int index) const
    {
        return Digit(index);
    }

    const Time& operator++()
    {
        assert(hour >= 0);
        assert(hour < 24);
        assert(minute >= 0);
        assert(minute < 60);

        if (++minute == 60)
        {
            minute = 0;

            if (++hour == 24)
            {
                hour = 0;
            }
        }

        return *this;
    }

    const Time operator++(int)
    {
        const auto result{ *this };
        ++(*this);
        return result;
    }

private:

    int hour;
    int minute;
};

inline 
bool operator == (const Time& left, const Time& right)
{
    return ((left.Hour() == right.Hour()) && (left.Minute() == right.Minute()));
}

inline 
bool operator != (const Time& left, const Time& right)
{
    return !(left == right);
}


template<int WIDTH, int HEIGHT>
struct DisplayRect
{
    int Width() const
    {
        return WIDTH;
    }

    int Height() const
    {
        return HEIGHT;
    }

protected:

    int Index(int x, int y) const
    {
        assert(x >= 0);
        assert(x < WIDTH);
        assert(y >= 0);
        assert(y < HEIGHT);

        return x + y * WIDTH;
    }
};


template<int WIDTH, int HEIGHT, typename TPixelType = char>
struct Display : DisplayRect<WIDTH, HEIGHT>
{
    explicit Display(TPixelType p = {})
    {
        Fill(p);
    }

    TPixelType Get(int x, int y) const
    {
        const auto index = this->Index(x, y);
        const auto p = pixel[index];

        return p;
    }

    void Set(int x, int y, TPixelType p)
    {
        const auto index = this->Index(x, y);
        pixel[index] = p;
    }

    void Fill(TPixelType p = {})
    {
        pixel.fill(p);
    }

private:

    std::array<TPixelType, WIDTH * HEIGHT> pixel;
};


template<int WIDTH, int HEIGHT, typename TPixelType>
inline
std::ostream& operator << (
    std::ostream& os, 
    const Display<WIDTH, HEIGHT, TPixelType>& display)
{
    for (int y = 0; y != HEIGHT; ++y)
    {
        for (int x = 0; x != WIDTH; ++x)
        {
            os << display.Get(x, y);
        }

        os << std::endl;
    }

    return os;
}


inline
void ReadDisplay(
    std::function<void(std::string&)> displayLineReader,
    std::function<void(int, int, char)> displaySet,
    int width,
    int height)
{
    std::string line;
    line.reserve(width); // TODO: get rid off allocation for every ReadDisplay...

    for (int y = 0; y != height; ++y)
    {
        displayLineReader(line);

        assert(line.length() >= width);

        for (int x = 0; x != width; ++x)
        {
            const char c = line[x];

            assert((c == '.') || (c == 'X'));

            displaySet(x, y, c);
        }
    }
}


template<typename Display>
inline
void ReadDisplay(std::function<void(std::string&)> displayLineReader, Display& display)
{
    const int width = display.Width();
    const int height = display.Height();

    auto displaySet = [&display](int x, int y, char p) { display.Set(x, y, p); };

    ReadDisplay(displayLineReader, displaySet, width, height);
}


struct DisplayReader
{
    explicit DisplayReader(std::function<void(std::string&)> displayLineReader) :
        displayLineReader(displayLineReader),
        displayIndex(0)
    {
    }

    template<typename Display>
    void operator()(Display& display)
    {
        if (displayIndex++ > 0)
        {
            std::string separatorLine;
            displayLineReader(separatorLine);

            assert(separatorLine.empty());
        }

        ReadDisplay(displayLineReader, display);
    }

private:

    std::function<void(std::string&)> displayLineReader;
    int displayIndex;
};


template<int WIDTH, int HEIGHT, int SEGMENTCOUNT>
struct DisplayPrimitiveDescription : DisplayRect<WIDTH, HEIGHT>
{
    DisplayPrimitiveDescription(std::array<int, WIDTH * HEIGHT> pixel) :
        pixel(pixel)
    {
    }

    int SegmentCount() const
    {
        return SEGMENTCOUNT;
    }

    int Get(int x, int y) const
    {
        const int index = this->Index(x, y);
        const auto p = pixel[index];

        return p;
    }

private:

    const std::array<int, WIDTH * HEIGHT> pixel;
};


template<typename DisplayPrimitiveDescription>
inline
void ExtractPrimitive(
    std::function<bool(int, int)> displayGet,
    const std::pair<int, int>& offset, 
    DisplayPrimitiveDescription& description,
    std::function<void(int,bool)> primitiveSet)
{
    const auto width = description.Width();
    const auto height = description.Height();

    for (int y = 0; y != height; ++y)
    {
        for (int x = 0; x != width; ++x)
        {
            const auto segmentIndex = description.Get(x, y);

            if (segmentIndex >= 0)
            {
                const auto displayX = x + offset.first;
                const auto displayY = y + offset.second;

                const auto segmentValue = displayGet(displayX, displayY);

                primitiveSet(segmentIndex, segmentValue);
            }
        }
    }
}


template<typename DisplayPrimitiveDescription>
inline
void WritePrimitive(
    std::function<char(int)> primitiveGet,
    const std::pair<int, int>& offset,
    DisplayPrimitiveDescription& description,
    std::function<void(int, int, char)> displaySet)
{
    const auto width = description.Width();
    const auto height = description.Height();

    for (int y = 0; y != height; ++y)
    {
        for (int x = 0; x != width; ++x)
        {
            const auto segmentIndex = description.Get(x, y);

            if (segmentIndex >= 0)
            {
                const auto segmentValue = primitiveGet(segmentIndex);

                const auto displayX = x + offset.first;
                const auto displayY = y + offset.second;

                displaySet(displayX, displayY, segmentValue);
            }
        }
    }
}


struct ClockBreaking
{
    using ClockDisplay = Display<21, 7>;

    static bool Solve(DisplayReader& displayReader, int displayCount, ClockDisplay& result)
    {
        assert(displayCount > 0);
//        assert(displayCount <= 100);

        const DisplayPrimitiveDescription<4, 7, 7> digitDisplayPrimitiveDescription(
        {
            -1,  0,  0, -1,
             1, -1, -1,  2,
             1, -1, -1,  2,
            -1,  3,  3, -1,
             4, -1, -1,  5,
             4, -1, -1,  5,
            -1,  6,  6, -1
        });

        const DisplayPrimitiveDescription<1, 7, 2> separatorDisplayPrimitiveDescription(
        {
            -1,
            -1,
             0,
            -1,
             1,
            -1,
            -1
        });

        enum : std::int8_t 
        {
            empty   = 0b00000000,
            zero    = 0b01110111,
            one     = 0b00100100,
            two     = 0b01011101,
            three   = 0b01101101,
            four    = 0b00101110,
            five    = 0b01101011,
            six     = 0b01111011,
            seven   = 0b00100101,
            eight   = 0b01111111,
            nine    = 0b01101111,

            _mask   = empty | zero | one | two | three | four | five | six | seven | eight | nine,
        };

        const std::pair<int, int> digitPrimitivePositions[4] =
        {
            { 17, 0 }, // Minute1
            { 12, 0 }, // Minute10
            { 5, 0 },  // Hour1
            { 0, 0 }   // Hour10
        };

        const std::pair<int, int> separatorPrimitivePosition = { 10, 0 };

        //

        std::vector<std::int32_t> digitDisplayPrimitivesList;
        digitDisplayPrimitivesList.reserve(displayCount);

        std::int32_t digitDisplayPrimitivesOr  = 0x00000000; // burn out candidates
        std::int32_t digitDisplayPrimitivesAnd = 0x00000000; // burn in candidates // 0xffffffff; // !

        std::int32_t separatorDisplayPrimitivesOr  = 0x00000000; // burn out candidates
        std::int32_t separatorDisplayPrimitivesAnd = 0x00000003; // burn in candidates

        ClockDisplay display;

        auto displayGet = [&display](int x, int y)
        {
            const char c = display.Get(x, y);
            assert((c == '.') || (c == 'X'));
            return (c == 'X');
        };

        for (int displayIndex = 0; displayIndex != displayCount; ++displayIndex)
        {
            // read next display

             displayReader(display);

            //

            // extract digit primitives

            // identify working segments, segments which might be burnt-in or burnt-out

            std::int32_t digitDisplayPrimitives = 0;
            
            auto* digitDisplayPrimitive = reinterpret_cast<std::int8_t*>(&digitDisplayPrimitives);
            for (const auto& digitPrimitivePosition : digitPrimitivePositions)
            {
                auto displayPrimitiveSet = [digitDisplayPrimitive](int segmentIndex, bool segmentValue)
                {
                    const auto mask = 1 << segmentIndex;

                    if (segmentValue)
                    {
                        *digitDisplayPrimitive |= mask;
                    }
                    else
                    {
                        *digitDisplayPrimitive &= ~mask;
                    }
                };

                ExtractPrimitive(displayGet, digitPrimitivePosition, digitDisplayPrimitiveDescription,
                    displayPrimitiveSet);

                //

                ++digitDisplayPrimitive;
            }

            digitDisplayPrimitivesList.push_back(digitDisplayPrimitives);

            digitDisplayPrimitivesOr |= digitDisplayPrimitives;
            if (displayIndex == 0)
            {
                digitDisplayPrimitivesAnd = digitDisplayPrimitives;
            }
            else
            {
                digitDisplayPrimitivesAnd &= digitDisplayPrimitives;
            }

            //

            // extract separator primitive

            {
                std::int32_t separatorDisplayPrimitive = 0;

                auto displayPrimitiveSet = [&separatorDisplayPrimitive](int segmentIndex, bool segmentValue)
                {
                    const auto mask = 1 << segmentIndex;

                    if (segmentValue)
                    {
                        separatorDisplayPrimitive |= mask;
                    }
                    else
                    {
                        separatorDisplayPrimitive &= ~mask;
                    }
                };

                ExtractPrimitive(displayGet, separatorPrimitivePosition, 
                    separatorDisplayPrimitiveDescription, displayPrimitiveSet);

                separatorDisplayPrimitivesOr |= separatorDisplayPrimitive;
                separatorDisplayPrimitivesAnd &= separatorDisplayPrimitive;
            }
        }

        //

        // defect segments are the one that are toggling

        const auto defectSeparatorDisplayPrimitiveSegments =
            separatorDisplayPrimitivesAnd ^ separatorDisplayPrimitivesOr;

        if (0 != defectSeparatorDisplayPrimitiveSegments)
        {
            return false;
        }

        //

        // digit primitives by position (index of primitive is its value)

        const std::array<std::vector<std::int8_t>, 4> digitValuePrimitives =
        {
            {
                { zero, one, two, three, four, five, six, seven, eight, nine }, // Minute1
                { zero, one, two, three, four, five }, // Minute10
                { zero, one, two, three, four, five, six, seven, eight, nine }, // Hour1
                { empty, one, two }, // Hour10
            }
        };
        
        auto asSegmentDisplay = [&digitValuePrimitives](const Time& time) -> std::int32_t
        {
            std::int32_t display = 0;

            auto p = reinterpret_cast<std::int8_t*>(&display);
            for (int digitIndex = 0; digitIndex != 4; ++digitIndex, ++p)
            {
                *p = digitValuePrimitives[digitIndex][time.Digit(digitIndex)];
            }

            return display;
        };

        // working segments are the one that are toggling

        const auto workingDigitDisplayPrimitivesSegments = 
            digitDisplayPrimitivesAnd ^ digitDisplayPrimitivesOr;


        // Detect for each display all the possible times.

        std::vector<std::pair<std::int32_t, std::int64_t>> times;
        times.reserve(displayCount);

        if (0 == workingDigitDisplayPrimitivesSegments)
        {
            // everything is possible...

            const std::pair<std::int32_t, std::int64_t> time = 
            {
                (1 << 24) - 1,
                (1ll << 60) - 1
            };

            for (int displayIndex = 0; displayIndex != displayCount; ++displayIndex)
            {
                times.push_back(time);
            }
        }
        else
        {
            for (int displayIndex = 0; displayIndex != displayCount; ++displayIndex)
            {
                std::array<int, 4> digitValues;

                auto workingDigitDisplayPrimitiveSegments = 
                    reinterpret_cast<const std::int8_t*>(&workingDigitDisplayPrimitivesSegments);

                const auto digitDisplayPrimitives = digitDisplayPrimitivesList[displayIndex];
                auto digitDisplayPrimitive = reinterpret_cast<const std::int8_t*>(&digitDisplayPrimitives);

                for (int digitIndex = 0; digitIndex != 4; ++digitIndex, ++digitDisplayPrimitive, ++workingDigitDisplayPrimitiveSegments)
                {
                    digitValues[digitIndex] = (1 << digitValuePrimitives[digitIndex].size()) - 1;

                    if (*workingDigitDisplayPrimitiveSegments != 0)
                    {
                        for (int digitValue = 0; digitValue != digitValuePrimitives[digitIndex].size(); ++digitValue)
                        {
                            const auto working = 
                                (0 == ((*digitDisplayPrimitive ^ digitValuePrimitives[digitIndex][digitValue]) & 
                                    *workingDigitDisplayPrimitiveSegments));

                            if (!working)
                            {
                                digitValues[digitIndex] &= ~(1 << digitValue);
                            }
                        }

                        if (digitValues[digitIndex] == 0)
                        {
                            return false;
                        }
                    }
                }


                std::int32_t hours = 0;
                std::int64_t minutes = 0;

                for (int hour10 = 0; hour10 != digitValuePrimitives[3].size(); ++hour10)
                {
                    if (0 != (digitValues[3] & (1 << hour10)))
                    {
                        for (int hour1 = 0; hour1 != digitValuePrimitives[2].size(); ++hour1)
                        {
                            if (0 != (digitValues[2] & (1 << hour1)))
                            {
                                const auto hour = hour10 * 10 + hour1;

                                if (hour < 24)
                                {
                                    hours |= 1 << hour;
                                }
                            }
                        }
                    }
                }

                if (hours == 0)
                {
                    return false;
                }

                for (int min10 = 0; min10 != digitValuePrimitives[1].size(); ++min10)
                {
                    if (0 != (digitValues[1] & (1 << min10)))
                    {
                        for (int min1 = 0; min1 != digitValuePrimitives[0].size(); ++min1)
                        {
                            if (0 != (digitValues[0] & (1 << min1)))
                            {
                                const auto minute = min10 * 10 + min1;

                                minutes |= 1ll << minute;
                            }
                        }
                    }
                }

                assert(minutes != 0);

                times.push_back(std::pair<std::int32_t, std::int64_t>(hours, minutes));
            }
        }
        
        //

        // Detect segments which are definitly burned in/out

//      std::vector<Time> startTimes;

        int startTimesCount = 0;
        
        std::int32_t segmentsAnd = 0;
        std::int32_t segmentsOr = 0;

        const auto& firstTime = times.front();
        for (int hour = 0; hour != 24; ++hour)
        {
            if (0 != (firstTime.first & (1 << hour)))
            {
                for (int minute = 0; minute != 60; ++minute)
                {
                    if (0 != (firstTime.second & (1ll << minute)))
                    {
                        const Time startTime(hour, minute);

                        std::int32_t _and = asSegmentDisplay(startTime);
                        std::int32_t _or = 0;

                        Time time(startTime);

                        bool ok = true;
                        for (const auto& t : times)
                        {
                            if ((0 == (t.first & (1 << time.Hour()))) ||
                                (0 == (t.second & (1ll << time.Minute()))))
                            {
                                ok = false;
                                break;
                            }

                            const auto segmentDisplay = asSegmentDisplay(time);

                            _and &= segmentDisplay;
                            _or |= segmentDisplay;

                            ++time;
                        }

                        if (ok)
                        {
                            if (startTimesCount == 0)
                            {
                                segmentsAnd = _and;
                                segmentsOr = _or;
                            }
                            else
                            {
                                segmentsAnd |= _and;
                                segmentsOr &= _or;
                            }

                            //

                            ++startTimesCount;

//                              startTimes.push_back(startTime);
                        }
                    }
                }
            }
        }

        if (startTimesCount == 0)
        {
            return false;
        }

        //

        const std::int8_t* pw = reinterpret_cast<const std::int8_t*>(&workingDigitDisplayPrimitivesSegments);

        const std::int8_t* biw = reinterpret_cast<const std::int8_t*>(&digitDisplayPrimitivesAnd);
        const std::int8_t* bow = reinterpret_cast<const std::int8_t*>(&digitDisplayPrimitivesOr);

        const std::int8_t* bi = reinterpret_cast<const std::int8_t*>(&segmentsAnd);
        const std::int8_t* bo = reinterpret_cast<const std::int8_t*>(&segmentsOr);

        result.Fill('.'); // ?

        auto displaySet = [&result](int x, int y, char c)
        {
            result.Set(x, y, c);
        };

        auto primitiveGet = [&pw, &biw, &bow, &bi, &bo](int segmentIndex) -> char
        {
            const auto mask = 1 << segmentIndex;
            const auto working = (0 != (*pw & mask));

            auto result = '?';
            if (working)
            {
                result = 'W';
            }
            else
            {
                if (0 != (*biw & mask))
                {
                    // burnt in or working

                    if (0 == (*bi & mask))
                    {
                        result = '1';
                    }
                }
                else // if (0 == (*bow & mask))
                {
                    // burnt out or working
                    
                    if (0 != (*bo & mask))
                    {
                        result = '0';
                    }
                }
            }

            return result;
        };

        for (int i = 0; i != 4; ++i, ++pw, ++biw, ++bow, ++bi, ++bo)
        {
            WritePrimitive(primitiveGet, digitPrimitivePositions[i],
                digitDisplayPrimitiveDescription, displaySet);
        }

        auto separatorPrimitiveGet = [separatorDisplayPrimitivesAnd](int segmentIndex) -> char
        {
            assert((segmentIndex == 0) || (segmentIndex == 1));

            const auto off = (0 == (separatorDisplayPrimitivesAnd & (1 << segmentIndex)));
            return off ? '0' : '?';
        };

        WritePrimitive(separatorPrimitiveGet, separatorPrimitivePosition,
            separatorDisplayPrimitiveDescription, displaySet);

        return true;
    }
};


void ClockBreaking(std::istream& input, std::ostream& output)
{
    for (;;)
    {
        auto displayLineReader = [&input](std::string& line) { std::getline(input, line); };

        std::string displayCountString;
        displayLineReader(displayCountString);

        if (displayCountString.empty())
        {
            break;
        }

        const auto displayCount = std::stoi(displayCountString);

        DisplayReader displayReader(displayLineReader);

        ClockBreaking::ClockDisplay result;
        const auto possible = ClockBreaking::Solve(displayReader, displayCount, result);
        if (possible)
        {
            output << result;
        }
        else
        {
            output << "impossible" << std::endl;
        }
    }
}


int
main(int argc, char* argv[])
{
    if (argc == 1)
    {
        ClockBreaking(std::cin, std::cout);
    }
    else
    {
        for (int index = 1; index != argc; ++index)
        {
            const char* fileName = argv[index];

            std::ifstream input;
            input.open(fileName);

            ClockBreaking(input, std::cout);
        }
    }

    return 0;
}
