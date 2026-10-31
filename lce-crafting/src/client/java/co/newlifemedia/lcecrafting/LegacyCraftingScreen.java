package co.newlifemedia.lcecrafting;

import co.newlifemedia.lcecrafting.mixin.SlotAccessor;
import net.minecraft.client.gui.GuiGraphicsExtractor;
import net.minecraft.client.gui.components.Button;
import net.minecraft.client.gui.screens.inventory.AbstractContainerScreen;
import net.minecraft.client.gui.screens.inventory.CraftingScreen;
import net.minecraft.client.gui.screens.inventory.InventoryScreen;
import net.minecraft.client.input.KeyEvent;
import net.minecraft.client.input.MouseButtonEvent;
import net.minecraft.core.component.DataComponents;
import net.minecraft.core.registries.BuiltInRegistries;
import net.minecraft.network.chat.Component;
import net.minecraft.util.context.ContextMap;
import net.minecraft.world.entity.player.Inventory;
import net.minecraft.world.entity.player.StackedItemContents;
import net.minecraft.world.inventory.*;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.crafting.RecipeBookCategories;
import net.minecraft.world.item.crafting.display.*;

import java.util.*;

/** Uses the existing menu and vanilla recipe placement; never creates or edits stacks. */
public final class LegacyCraftingScreen<T extends AbstractCraftingMenu> extends AbstractContainerScreen<T> {
    public static boolean bypassReplacement;
    private static final String[] CATEGORIES = {"Structures", "Tools", "Food", "Armour", "Mechanisms", "Transport", "Decoration"};
    private final Inventory inventory;
    private final Map<Slot, int[]> originalPositions = new IdentityHashMap<>();
    private final StackedItemContents contents = new StackedItemContents();
    private List<List<RecipeDisplayEntry>> families = List.of();
    private ContextMap context;
    private int category, family, variant, page, refreshTicks;
    private RecipeDisplayEntry pending;
    private boolean pendingAll;
    private int pendingState, pendingTicks, settledTicks;
    private boolean switching;
    private String status = "Select a recipe";
    private Button craft, craftAll;

    public LegacyCraftingScreen(T menu, Inventory inventory) {
        super(menu, inventory, Component.literal("Crafting"), 300, 232);
        this.inventory = inventory;
    }

    @Override protected void init() {
        super.init();
        context = SlotDisplayContext.fromLevel(minecraft.level);
        arrangeSlots();
        for (int i = 0; i < CATEGORIES.length; i++) {
            final int tab = i;
            addRenderableWidget(Button.builder(Component.literal(CATEGORIES[i].substring(0, 4)), b -> {
                category = tab; family = variant = page = 0; refreshRecipes();
            }).bounds(leftPos + 8 + i * 41, topPos + 17, 40, 18).build());
        }
        addRenderableWidget(Button.builder(Component.literal("<"), b -> changePage(-1))
                .bounds(leftPos + 261, topPos + 40, 14, 20).build());
        addRenderableWidget(Button.builder(Component.literal(">"), b -> changePage(1))
                .bounds(leftPos + 277, topPos + 40, 14, 20).build());
        craft = addRenderableWidget(Button.builder(Component.literal("Craft"), b -> requestCraft(false))
                .bounds(leftPos + 218, topPos + 110, 72, 20).build());
        craftAll = addRenderableWidget(Button.builder(Component.literal("Craft all"), b -> requestCraft(true))
                .bounds(leftPos + 218, topPos + 134, 72, 20).build());
        addRenderableWidget(Button.builder(Component.literal("Java menu"), b -> openVanilla())
                .bounds(leftPos + 218, topPos + 185, 72, 20).build());
        refreshRecipes();
    }

    private void position(Slot slot, int x, int y) {
        originalPositions.putIfAbsent(slot, new int[]{slot.x, slot.y});
        var access = (SlotAccessor) slot;
        access.lce$setX(x); access.lce$setY(y);
    }

    private void arrangeSlots() {
        // Retain slot identities, indices and vanilla click/drag handling.
        for (Slot slot : menu.slots) {
            if (slot.container == inventory) {
                int index = slot.getContainerSlot();
                if (index < 9) position(slot, 8 + index * 18, 210);
                else if (index < 36) position(slot, 8 + (index - 9) % 9 * 18, 150 + (index - 9) / 9 * 18);
                else if (index >= 36 && index <= 39) position(slot, 190, 150 + (39 - index) * 18);
                else position(slot, 218, 164);
            }
        }
        int index = 0;
        for (Slot slot : menu.getInputGridSlots()) {
            position(slot, 104 + index % menu.getGridWidth() * 18, 77 + index / menu.getGridWidth() * 18);
            index++;
        }
        position(menu.getResultSlot(), 194, 95);
    }

    private void refreshRecipes() {
        if (minecraft.player == null) return;
        contents.clear();
        inventory.fillStackedContents(contents);
        menu.fillCraftSlotsStackedContents(contents);
        Map<String, List<RecipeDisplayEntry>> grouped = new TreeMap<>();
        for (var collection : minecraft.player.getRecipeBook().getCollections()) {
            for (var recipe : collection.getRecipes()) {
                if (!fits(recipe.display())) continue;
                ItemStack output = output(recipe);
                if (output.isEmpty() || categoryOf(recipe, output) != category) continue;
                String key = familyKey(recipe, output);
                grouped.computeIfAbsent(key, k -> new ArrayList<>()).add(recipe);
            }
        }
        families = new ArrayList<>(grouped.values());
        family = Math.clamp(family, 0, Math.max(0, families.size() - 1));
        page = Math.clamp(page, 0, Math.max(0, (families.size() - 1) / 10));
        variant = Math.clamp(variant, 0, families.isEmpty() ? 0 : families.get(family).size() - 1);
        updateButtons();
    }

    private boolean fits(RecipeDisplay display) {
        if (display instanceof ShapedCraftingRecipeDisplay shaped)
            return shaped.width() <= menu.getGridWidth() && shaped.height() <= menu.getGridHeight();
        return display instanceof ShapelessCraftingRecipeDisplay shapeless
                && shapeless.ingredients().size() <= menu.getGridWidth() * menu.getGridHeight();
    }

    private ItemStack output(RecipeDisplayEntry entry) {
        return entry.display().result().resolveForFirstStack(context);
    }

    private static String itemId(ItemStack stack) {
        return BuiltInRegistries.ITEM.getKey(stack.getItem()).toString();
    }

    private static String familyKey(RecipeDisplayEntry entry, ItemStack output) {
        String id = itemId(output);
        // Match LCE's vertical material/colour variants. Server recipe groups remain authoritative otherwise.
        for (String suffix : List.of("pickaxe", "shovel", "sword", "axe", "hoe", "helmet", "chestplate", "leggings", "boots", "planks", "stairs", "slab", "wool", "carpet", "boat", "door", "fence"))
            if (id.endsWith("_" + suffix)) return "family:" + suffix;
        return entry.group().isPresent() ? "group:" + entry.group().getAsInt() : id;
    }

    private static int categoryOf(RecipeDisplayEntry entry, ItemStack output) {
        String id = itemId(output);
        if (id.contains("minecart") || id.contains("rail") || id.endsWith("boat") || id.endsWith("raft")) return 5;
        if (output.has(DataComponents.FOOD)) return 2;
        if (id.endsWith("helmet") || id.endsWith("chestplate") || id.endsWith("leggings") || id.endsWith("boots")) return 3;
        if (entry.category() == RecipeBookCategories.CRAFTING_EQUIPMENT) return 1;
        if (entry.category() == RecipeBookCategories.CRAFTING_REDSTONE) return 4;
        if (entry.category() == RecipeBookCategories.CRAFTING_BUILDING_BLOCKS) return 0;
        return 6;
    }

    private RecipeDisplayEntry selected() {
        return families.isEmpty() ? null : families.get(family).get(variant);
    }

    private void changePage(int direction) {
        page = Math.floorMod(page + direction, Math.max(1, (families.size() + 9) / 10));
        family = page * 10; variant = 0;
    }

    private void updateButtons() {
        if (craft == null) return;
        var recipe = selected();
        boolean available = pending == null && recipe != null && recipe.canCraft(contents) && menu.getCarried().isEmpty();
        craft.active = craftAll.active = available;
    }

    private void requestCraft(boolean all) {
        refreshRecipes();
        var recipe = selected();
        if (pending != null || recipe == null || !recipe.canCraft(contents) || !menu.getCarried().isEmpty()
                || minecraft.gameMode == null || minecraft.player.containerMenu != menu) return;
        pending = recipe;
        pendingAll = all;
        pendingState = menu.getStateId(); pendingTicks = settledTicks = 0;
        status = "Waiting for server...";
        minecraft.gameMode.handlePlaceRecipe(menu.containerId, recipe.id(), all);
        updateButtons();
    }

    @Override protected void containerTick() {
        super.containerTick();
        if (++refreshTicks % 5 == 0) refreshRecipes();
        if (pending == null) return;
        if (minecraft.player == null || minecraft.player.containerMenu != menu || !menu.getCarried().isEmpty()) {
            pending = null; status = "Craft cancelled"; updateButtons(); return;
        }
        // Wait for a server update, then allow its slot updates to settle before taking the result.
        if (menu.getStateId() != pendingState && ++settledTicks >= 2) {
            ItemStack result = menu.getResultSlot().getItem();
            ItemStack expected = output(pending);
            if (!result.isEmpty() && ItemStack.isSameItemSameComponents(result, expected)
                    && result.getCount() == expected.getCount()) {
                minecraft.gameMode.handleContainerInput(menu.containerId, menu.getResultSlot().index, 0,
                        pendingAll ? ContainerInput.QUICK_MOVE : ContainerInput.PICKUP, minecraft.player);
                if (!pendingAll) depositCraftedStack();
                pending = null; status = "Craft requested"; refreshRecipes(); return;
            }
        }
        if (++pendingTicks >= 100) {
            pending = null; status = "No result received. Check inventory space."; updateButtons();
        }
    }

    private void depositCraftedStack() {
        // One result pickup crafts exactly once, even if the grid already holds multiple sets.
        // Use normal slot clicks to store it. A full inventory leaves it safely on the cursor.
        for (Slot slot : menu.slots) {
            ItemStack carried = menu.getCarried();
            if (carried.isEmpty()) break;
            if (slot.container != inventory || slot.getContainerSlot() >= 36 || !slot.mayPlace(carried)) continue;
            ItemStack existing = slot.getItem();
            if (existing.isEmpty() || (ItemStack.isSameItemSameComponents(existing, carried)
                    && existing.getCount() < slot.getMaxStackSize(carried))) {
                minecraft.gameMode.handleContainerInput(menu.containerId, slot.index, 0, ContainerInput.PICKUP, minecraft.player);
            }
        }
    }

    @Override public boolean mouseClicked(MouseButtonEvent event, boolean doubleClick) {
        double x = event.x() - leftPos, y = event.y() - topPos;
        if (event.button() == 0 && y >= 40 && y < 62 && x >= 8 && x < 258) {
            int target = page * 10 + (int) ((x - 8) / 25);
            if (target < families.size()) { family = target; variant = 0; updateButtons(); }
            return true;
        }
        if (event.button() == 0 && x >= 20 && x < 42 && y >= 75 && y < 135 && !families.isEmpty()) {
            variant = Math.floorMod(variant + (y < 95 ? -1 : 1), families.get(family).size());
            updateButtons(); return true;
        }
        // Do not allow manual slot edits while an automatic craft is awaiting acknowledgement.
        if (pending != null) return true;
        return super.mouseClicked(event, doubleClick);
    }

    @Override public boolean keyPressed(KeyEvent event) {
        if (pending != null && event.key() != 256) return true;
        switch (event.key()) {
            case 263, 262 -> {
                if (!families.isEmpty()) {
                    family = Math.floorMod(family + (event.key() == 263 ? -1 : 1), families.size());
                    variant = 0; page = family / 10; updateButtons();
                }
                return true;
            }
            case 265, 264 -> {
                if (!families.isEmpty()) variant = Math.floorMod(variant + (event.key() == 265 ? -1 : 1), families.get(family).size());
                updateButtons(); return true;
            }
            case 257, 335 -> { requestCraft(event.hasShiftDown()); return true; }
            default -> { return super.keyPressed(event); }
        }
    }

    @Override public boolean mouseScrolled(double x, double y, double horizontal, double vertical) {
        if (y < topPos + 140 && !families.isEmpty()) {
            variant = Math.floorMod(variant + (vertical > 0 ? -1 : 1), families.get(family).size());
            updateButtons(); return true;
        }
        return super.mouseScrolled(x, y, horizontal, vertical);
    }

    @Override public void extractBackground(GuiGraphicsExtractor g, int mouseX, int mouseY, float delta) {
        g.fill(leftPos, topPos, leftPos + imageWidth, topPos + imageHeight, 0xFFBEBEBE);
        g.outline(leftPos, topPos, imageWidth, imageHeight, 0xFFFFFFFF);
        for (Slot slot : menu.slots) {
            g.fill(leftPos + slot.x - 1, topPos + slot.y - 1, leftPos + slot.x + 17, topPos + slot.y + 17, 0xFF777777);
        }
        g.text(font, CATEGORIES[category], leftPos + 8, topPos + 5, 0xFF303030, false);
        for (int i = 0; i < 10 && page * 10 + i < families.size(); i++) {
            int index = page * 10 + i;
            var entry = families.get(index).getFirst();
            int x = leftPos + 8 + i * 25, y = topPos + 40;
            g.fill(x, y, x + 23, y + 22, index == family ? 0xFFFFFFFF : 0xFF999999);
            g.item(output(entry), x + 3, y + 3);
        }
        var entry = selected();
        if (entry != null) {
            ItemStack result = output(entry);
            g.text(font, font.plainSubstrByWidth(result.getHoverName().getString(), 180), leftPos + 70, topPos + 65, 0xFF303030, false);
            var variants = families.get(family);
            for (int i = -1; i <= 1; i++) {
                var option = variants.get(Math.floorMod(variant + i, variants.size()));
                int x = leftPos + 20, y = topPos + 96 + i * 20;
                g.fill(x - 1, y - 1, x + 17, y + 17, i == 0 ? 0xFFFFFFFF : 0xFF999999);
                g.item(output(option), x, y);
            }
            // Ghost ingredients appear only in empty real input slots.
            List<SlotDisplay> ingredients = entry.display() instanceof ShapedCraftingRecipeDisplay shaped
                    ? shaped.ingredients() : ((ShapelessCraftingRecipeDisplay) entry.display()).ingredients();
            int recipeWidth = entry.display() instanceof ShapedCraftingRecipeDisplay shaped ? shaped.width() : menu.getGridWidth();
            for (int i = 0; i < ingredients.size(); i++) {
                int gridIndex = i / recipeWidth * menu.getGridWidth() + i % recipeWidth;
                Slot slot = menu.getInputGridSlots().get(gridIndex);
                if (!slot.hasItem()) {
                    g.fakeItem(ingredients.get(i).resolveForFirstStack(context), leftPos + slot.x, topPos + slot.y);
                    g.fill(leftPos + slot.x, topPos + slot.y, leftPos + slot.x + 16, topPos + slot.y + 16, 0x66888888);
                }
            }
            if (!menu.getResultSlot().hasItem()) g.fakeItem(result, leftPos + 194, topPos + 95);
            if (!entry.canCraft(contents)) g.outline(leftPos + 192, topPos + 93, 20, 20, 0xFFB53131);
        }
        g.text(font, "Inventory", leftPos + 8, topPos + 138, 0xFF303030, false);
        g.text(font, font.plainSubstrByWidth(status, 178), leftPos + 112, topPos + 5, 0xFF303030, false);
    }

    @Override public void extractRenderState(GuiGraphicsExtractor g, int mouseX, int mouseY, float delta) {
        super.extractRenderState(g, mouseX, mouseY, delta);
        int x = mouseX - leftPos, y = mouseY - topPos;
        if (y >= 40 && y < 62 && x >= 8 && x < 258) {
            int index = page * 10 + (x - 8) / 25;
            if (index < families.size()) g.setTooltipForNextFrame(font, output(families.get(index).getFirst()), mouseX, mouseY);
        }
    }

    @Override protected void extractLabels(GuiGraphicsExtractor g, int mouseX, int mouseY) {}

    private void restorePositions() {
        originalPositions.forEach((slot, xy) -> {
            ((SlotAccessor) slot).lce$setX(xy[0]); ((SlotAccessor) slot).lce$setY(xy[1]);
        });
        originalPositions.clear();
    }

    private void openVanilla() {
        if (pending != null) return;
        switching = true;
        restorePositions();
        bypassReplacement = true;
        try {
            if (menu instanceof CraftingMenu table) minecraft.gui.setScreen(new CraftingScreen(table, inventory, title));
            else minecraft.gui.setScreen(new InventoryScreen(minecraft.player));
        } finally { bypassReplacement = false; switching = false; }
    }

    @Override public void removed() {
        pending = null;
        restorePositions();
        if (!switching) super.removed();
    }
}
