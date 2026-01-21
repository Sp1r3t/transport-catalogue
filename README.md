# Transport Catalogue

**Transport Catalogue** is a comprehensive C++ system for managing municipal transport data, processing route queries, and generating geographical maps. This project implements a complete backend for a transport directory, including route optimization and SVG visualization.

---

## Examples

![Create SVG Output](images/SVG%20Output.png)

## Core Features

### 1. Transport Database
- Efficient storage of stops (coordinates) and bus routes (circular or linear).  
- Quick lookups for bus information: number of stops, unique stops, route length, and curvature (efficiency ratio).  
- Tracking of all buses passing through a specific stop.

### 2. Route Planning & Optimization
- Built-in graph-based router using **Dijkstra's algorithm**.  
- Calculation of optimal travel time considering both waiting time at stops and travel time between them.  
- Support for **road distances** (real path length) versus **geographical distances**.

### 3. Data Processing (JSON)
- Custom JSON library for parsing input configurations and generating structured reports.  
- **JSON Builder:** A chainable API utilizing the **Fluent Interface** pattern to create complex JSON structures safely at compile-time.

### 4. Map Rendering
- Visualizes transport routes as an SVG image.  
- Customizable rendering settings: palette colors, line widths, font sizes, and offsets.  
- Automated coordinate projection (**SphereProjector**) to map geographical coordinates onto a 2D plane.

---

## Technical Architecture

- **TransportCatalogue:** Core engine managing relationships between stops and buses.  
- **MapRenderer:** Converts geographical data into SVG graphical primitives.  
- **TransportRouter:** Handles graph construction and route weight calculations.  
- **RequestHandler:** Acts as a **facade** to coordinate data flow between the catalogue, renderer, and router.  
- **JsonReader:** Orchestrates parsing of input requests and formatting of output responses.

---

## Technical Stack

- **Language:** C++17 / C++20  
- **Format Support:** JSON (input/output), SVG (visualization)  
- **Algorithms:** Dijkstra for shortest paths, coordinate projection for mapping  
- **Design Patterns:** Builder (for JSON), Facade (Request Handler)

---

## Project Structure

| File | Description |
|------|-------------|
| `transport_catalogue.cpp` | Core logic for data storage and retrieval. |
| `transport_router.cpp` | Graph construction and routing logic. |
| `map_renderer.cpp` | SVG generation and coordinate projection. |
| `json_builder.cpp` | Safe JSON construction using a state-based builder. |
| `request_handler.cpp` | Interface between the database and visualization/routing modules. |
| `svg.cpp` | Basic SVG object library (Circle, Polyline, Text). |

---

## Usage

The system processes a JSON input containing:

- **base_requests:** Data to populate the catalogue (stops and buses).  
- **render_settings:** Visual parameters for the map.  
- **routing_settings:** Parameters like bus wait time and velocity.  
- **stat_requests:** Queries for bus info, stop info, map rendering, or optimal routing.

**Example Workflow:**
1. Populate the catalogue with stops and buses from JSON input.  
2. Build a graph for routing and calculate optimal paths.  
3. Render the transport map to SVG based on user settings.  
4. Respond to queries about buses, stops, or routes in structured JSON format.

