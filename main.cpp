#include "raylib.h"

#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <math.h>
#include <sstream>
#include <string>
#include <fstream>
#include <cassert>
#include <thread>
struct MarketData
{
    std::mutex mtx;                      
    std::vector<float> priceHistory;     
    std::map<double, int> orderBookBids; 

    std::map<double, int> orderBookAsks; 
    std::vector<float> open;
    std::vector<float> close;
    std::vector<float> high;
    std::vector<float> low;
    double lastPrice = 0.0;
};

MarketData g_marketData;
std::vector<std::string> parseCSVLine(const std::string &line, char delimiter = ',')
{
    std::vector<std::string> fields;
    std::stringstream ss(line); 
    std::string field;

    while (std::getline(ss, field, delimiter))
    {
        fields.push_back(field);
    }
    return fields;
}
void readCSV(const std::string &filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open the file " << filename << std::endl;
        return;
    }

    std::string line;
    int line_count = 0;

    while (std::getline(file, line))
    {
        line_count++;

        if (line.empty())
        {
            continue;
        }

        std::vector<std::string> fields = parseCSVLine(line);

     
        for (size_t i = 1; i+3< fields.size(); i += 6)
        {
            try
            {
                // 1. Open
                float open_f = std::stof(fields[i]);
                g_marketData.open.push_back(open_f);

                // 2. High
                float high_f = std::stof(fields[i + 1]);
                g_marketData.high.push_back(high_f);

                // 3. Low
                float low_f = std::stof(fields[i + 2]);
                g_marketData.low.push_back(low_f);

                // 4. Close
                float close_f = std::stof(fields[i + 3]);
                g_marketData.close.push_back(close_f);
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Market data parse error at index " << i << ": Invalid argument - " << e.what() << std::endl;
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "Market data parse error at index " << i << ": Out of range - " << e.what() << std::endl;
            }
        }
        for(int j =0; j<g_marketData.close.size(); j++){
            
            std::cout << g_marketData.open[j] << " " << g_marketData.high[j] << " " << g_marketData.low[j] << " " << g_marketData.close[j] << std::endl;
        }

    }

    file.close();
}

const int screenWidth = 1600;
const int screenHeight = 900;

const Rectangle CHART_AREA = {50, 50, 1000, 500};
const Rectangle ORDERBOOK_AREA = {1100, 50, 450, 500};
const Rectangle CONTROL_AREA = {50, 600, 700, 250};
const Rectangle LOG_AREA = {800, 600, 750, 250};


void DrawChartPanel(MarketData &data, Font customFont)
{
    DrawRectangleRec(CHART_AREA, Fade(BLUE, 0.1f));
    DrawRectangleLinesEx(CHART_AREA, 1, BLUE);
    DrawTextEx(customFont, "REAL-TIME CHART (1M)", (Vector2){CHART_AREA.x + 10, CHART_AREA.y + 10}, 30, 2, LIGHTGRAY);


    std::lock_guard<std::mutex> lock(data.mtx);


    if (data.priceHistory.size() > 1)
    {
        float xStep = CHART_AREA.width / (float)data.priceHistory.size();
        float priceMin = data.priceHistory[0];
        float priceMax = data.priceHistory[0];
        for (float p : data.priceHistory)
        {
            if (p < priceMin)
                priceMin = p;
            if (p > priceMax)
                priceMax = p;
        }

        
        float priceRange = priceMax - priceMin;
        float yScaling = CHART_AREA.height * 0.9f / (priceRange > 0.01 ? priceRange : 1.0f);

        for (size_t i = 0; i < data.priceHistory.size() - 1; ++i)
        {
            Vector2 p1, p2;

 
            p1.x = CHART_AREA.x + (float)i * xStep;
            p2.x = CHART_AREA.x + (float)(i + 1) * xStep;

            p1.y = CHART_AREA.y + CHART_AREA.height - 20 - (data.priceHistory[i] - priceMin) * yScaling;
            p2.y = CHART_AREA.y + CHART_AREA.height - 20 - (data.priceHistory[i + 1] - priceMin) * yScaling;

            DrawLineV(p1, p2, GREEN);
        }
    }
}

// Function to draw the Order Book
void DrawOrderBookPanel(MarketData &data)
{
    DrawRectangleRec(ORDERBOOK_AREA, Fade(BROWN, 0.1f));
    DrawRectangleLinesEx(ORDERBOOK_AREA, 1, BROWN);
    DrawText("ORDER BOOK", ORDERBOOK_AREA.x + 10, ORDERBOOK_AREA.y + 10, 20, LIGHTGRAY);

    std::lock_guard<std::mutex> lock(data.mtx);

    float startY = ORDERBOOK_AREA.y + 40;
    float rowHeight = 20;


    DrawText("ASKS", ORDERBOOK_AREA.x + 20, startY, 18, RED);
    startY += rowHeight;
    int row = 0;
    for (auto it = data.orderBookAsks.rbegin(); it != data.orderBookAsks.rend() && row < 10; ++it, ++row)
    {
        DrawText(TextFormat("%-10.2f %d", it->first, it->second), ORDERBOOK_AREA.x + 20, startY + row * rowHeight, 16, RED);
    }

    startY += rowHeight * 11;


    DrawText(TextFormat("LAST: %.2f", data.lastPrice), ORDERBOOK_AREA.x + 20, startY, 22, YELLOW);
    startY += rowHeight * 2;

    DrawText("BIDS", ORDERBOOK_AREA.x + 20, startY, 18, GREEN);
    startY += rowHeight;
    row = 0;
 
    for (auto const &[price, volume] : data.orderBookBids)
    {
        if (row >= 10)
            break;
        DrawText(TextFormat("%-10.2f %d", price, volume), ORDERBOOK_AREA.x + 20, startY + row * rowHeight, 16, GREEN);
        row++;
    }
}

void DrawControlAndLogPanels()
{

    DrawRectangleRec(LOG_AREA, Fade(DARKGRAY, 0.1f));
    DrawRectangleLinesEx(LOG_AREA, 1, DARKGRAY);
    DrawText("SYSTEM LOG", LOG_AREA.x + 10, LOG_AREA.y + 10, 20, SKYBLUE);

    DrawText("> Loading data...", LOG_AREA.x + 20, LOG_AREA.y + 50, 16, LIGHTGRAY);

    DrawText(TextFormat("USER ID: %s", "DEV"), LOG_AREA.x + 20, LOG_AREA.y + 200, 14, GRAY);
}


int main(void)
{

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "delta v0.1");
    SetTargetFPS(60);
    Font customFont = LoadFont("/home/user1/Documents/fonts/ttf/JetBrainsMono-Regular.ttf");
    std::thread t(readCSV, "fake_btc_data.csv");
    while (!WindowShouldClose())
    {
   

        BeginDrawing();
        ClearBackground(Color{20, 20, 25, 255});

        DrawChartPanel(g_marketData, customFont);
        DrawOrderBookPanel(g_marketData);
        DrawControlAndLogPanels();

        DrawFPS(10, 10);

        EndDrawing();
    }


    t.join();
    CloseWindow();
    UnloadFont(customFont);
    return 0;
}